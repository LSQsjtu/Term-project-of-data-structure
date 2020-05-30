#include "utility.hpp"
#include <functional>
#include <cstddef>
#include <cstring>
#include "exception.hpp"

namespace sjtu
{
    template <class Key, class Value>
    class BTree
    {
    public:
        class iterator;
        typedef pair<Key, Value> value_type;

    private:
        // Your private members go here加上一个最小键，方便看是否超出
        static const int M = 4080 / (sizeof(size_t) + sizeof(Key)); //4096-sizeof(bool) - sizeof(int) - sizeof(size_t)-sizeof(off_t)
        static const int L = 4080 / (sizeof(value_type));           //4096-sizeof(int)-sizeof(size_t)*3-sizeof(off_t)
        static const int Mmin = M / 2;
        static const int Lmin = L / 2;
        static const int info_offset = 0;
        struct name
        {
            char *str;
            name() { str = new char[10]; }
            ~name() { delete str; }
            void naming(const char *oth) { strcpy(str, oth); }
        };

        struct TreeInfo
        {
            size_t head; //head of leaf
            size_t tail; //tail of leaf
            size_t root; //root of BTree
            size_t size; //size of BTree
            off_t eof;   //end of file

            TreeInfo()
            {
                head = 0;
                tail = 0;
                root = 0;
                size = 0;
                eof = 0;
            }
        };

        struct leafNode
        {
            off_t offset;
            size_t parent;
            size_t pre;
            size_t next;
            int cnt; //number of pairs in leaf
            value_type data[L];
            leafNode()
            {
                offset = 0;
                parent = 0;
                cnt = 0;
            }
        };

        struct internalNode
        {
            off_t offset;
            size_t parent;
            size_t child[M];
            Key key[M];
            int cnt;
            bool type; // child is leaf or not
            internalNode()
            {
                offset = 0;
                parent = 0;
                for (int i = 0; i < M; ++i)
                    child[i] = 0;
                cnt = 0;
                type = 0;
            }
        };

        TreeInfo info;
        FILE *file;
        name filename;
        bool fileOpen = false;
        bool fileExist = false;

        void openFile()
        {
            fileExist = 1;
            if (fileOpen == 0)
            {
                file = fopen(filename.str, "rb+");
                if (file == nullptr)
                {
                    fileExist = 0;
                    file = fopen(filename.str, "wb+");
                }
                else
                    fileRead(&info, info_offset, 1, sizeof(TreeInfo));
                fileOpen = 1;
            }
        }

        void closeFile()
        {
            if (fileOpen == 1)
                fclose(file);
            fileOpen = 0;
        }
        //place指针，设为void，可以写入任意类型;offset：在文件中读取的位置;num:向后读多少个
        void fileRead(void *place, off_t offset, size_t num, size_t size) const
        {
            fseek(file, offset, SEEK_SET);
            fread(place, size, num, file);
        }

        void fileWrite(void *place, off_t offset, size_t num, size_t size) const
        {
            fseek(file, offset, SEEK_SET);
            fwrite(place, size, num, file);
        }

        FILE *filefrom;
        name filefromName;

        void copy_fileRead(void *place, off_t offset, size_t num, size_t size) const
        {
            fseek(filefrom, offset, SEEK_SET);
            fread(place, size, num, filefrom);
        }

        off_t prevLeaf;                                                      // 前一个叶子
        void copy_leaf(off_t offset, off_t from_offset, off_t parent_offset) //在复制函数中会用到位置，from叶子的位置，from叶子的父亲的地址
        {
            leafNode leaf, leafFrom, preleaf;
            copy_fileRead(&leafFrom, from_offset, 1, sizeof(leafNode));

            leaf.offset = offset;
            leaf.parent = parent_offset;
            leaf.cnt = leafFrom.cnt;
            leaf.pre = prevLeaf;
            leaf.next = 0;
            if (prevLeaf != 0) //当前复制的非头叶子
            {
                fileRead(&preleaf, prevLeaf, 1, sizeof(leafNode));
                preleaf.next = offset; //更换当前leaf的位
                fileWrite(&preleaf, prevLeaf, 1, sizeof(leafNode));
                info.tail = offset;
            }
            else
                info.head = offset; //加入头叶子

            for (int i = 0; i < leaf.cnt; i++)
            {
                leaf.data[i].first = leafFrom.data[i].first;
                leaf.data[i].second = leafFrom.data[i].second;
            }

            fileWrite(&leaf, offset, 1, sizeof(leafNode));

            info.eof += sizeof(leafNode);
            prevLeaf = offset; //为下一个叶子的连接做准备
        }

        void copy_node(off_t offset, off_t from_offset, off_t parent_offset)
        {
            internalNode node, node_from;
            copy_fileRead(&node_from, from_offset, 1, sizeof(internalNode));
            fileWrite(&node, offset, 1, sizeof(internalNode));

            info.eof += sizeof(internalNode);
            node.offset = offset;
            node.parent = parent_offset;
            node.cnt = node_from.cnt;
            node.type = node_from.type;

            for (int i = 0; i < node.cnt; i++)
            {
                node.key[i] = node_from.key[i];
                if (node.type == 1)
                    copy_leaf(info.eof, node_from.child[i], offset); //在文件尾连入叶子
                else
                    copy_node(info.eof, node_from.child[i], offset);
            }

            fileWrite(&node, offset, 1, sizeof(internalNode));
        }

        size_t locate_leaf(const Key &key, off_t offset) const //返回leaf.offset，从root开始找，key[0]为最小值
        {
            internalNode p;
            fileRead(&p, offset, 1, sizeof(internalNode));

            if (p.type == 1)
            {
                int pos = 0;
                for (; pos < p.cnt; pos++)
                    if (key < p.key[pos])
                        break;
                if (pos == 0) //key[0]之前
                    return 0;
                return p.child[pos - 1];
            }
            else
            {
                int pos;
                for (pos = 0; pos < p.cnt; pos++)
                    if (key < p.key[pos])
                        break;
                if (pos == 0)
                    return 0;
                return locate_leaf(key, p.child[pos - 1]);
            }
        }

        bool insertLeaf(leafNode &leaf, const Key &key, const Value &value)
        {
            iterator ret;
            int pos = 0;

            for (; pos < leaf.cnt; ++pos) //找到key位于pos - 1到pos之间
            {
                if (key == leaf.data[pos].first)
                    return false; //已经存在
                if (key < leaf.data[pos].first)
                    break;
            }

            for (int i = leaf.cnt - 1; i >= pos; --i)
            {
                leaf.data[i + 1].first = leaf.data[i].first;
                leaf.data[i + 1].second = leaf.data[i].second;
            }
            leaf.data[pos].first = key;
            leaf.data[pos].second = value;

            ++leaf.cnt;
            ++info.size;
            ret.tree = this;
            ret.offset = leaf.offset;
            ret.place = pos;

            fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
            if (leaf.cnt < L)
                fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
            else
                splitLeaf(leaf, ret, key);
            return true;
        }

        void insertNode(internalNode &node, const Key &key, size_t ch)
        {
            int pos = 0;
            for (; pos < node.cnt; pos++)
                if (key < node.key[pos])
                    break;

            for (int i = node.cnt - 1; i >= pos; i--)
            {
                node.key[i + 1] = node.key[i];
                node.child[i + 1] = node.child[i];
            }
            node.key[pos] = key;
            node.child[pos] = ch;
            node.cnt++;

            if (node.cnt < M)
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
            else
                splitNode(node);
        }

        void splitLeaf(leafNode &leaf, iterator &it, const Key &key)
        {
            leafNode newleaf;
            newleaf.cnt = leaf.cnt - (leaf.cnt / 2);
            leaf.cnt = leaf.cnt / 2;
            newleaf.parent = leaf.parent;
            newleaf.offset = info.eof; //尾部增加叶子
            info.eof += sizeof(leafNode);

            for (int i = 0; i < newleaf.cnt; i++)
            {
                newleaf.data[i].first = leaf.data[i + leaf.cnt].first;
                newleaf.data[i].second = leaf.data[i + leaf.cnt].second;
                if (newleaf.data[i].first == key)
                {
                    it.offset = newleaf.offset;
                    it.place = i;
                }
            }

            newleaf.next = leaf.next;
            newleaf.pre = leaf.offset;
            leaf.next = newleaf.offset;
            leafNode nextleaf;
            if (newleaf.next == 0)
                info.tail = newleaf.offset; //改尾部
            else
            { //中间插入
                fileRead(&nextleaf, newleaf.next, 1, sizeof(leafNode));
                nextleaf.pre = newleaf.offset;
                fileWrite(&nextleaf, nextleaf.offset, 1, sizeof(leafNode));
            }

            fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
            fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
            fileWrite(&newleaf, newleaf.offset, 1, sizeof(leafNode));

            internalNode parent;
            fileRead(&parent, leaf.parent, 1, sizeof(internalNode));
            insertNode(parent, newleaf.data[0].first, newleaf.offset);
        }

        void splitNode(internalNode &node)
        {
            internalNode newnode;
            newnode.cnt = node.cnt - node.cnt / 2;
            node.cnt = node.cnt / 2;
            newnode.parent = node.parent;
            newnode.type = node.type;
            newnode.offset = info.eof;
            info.eof += sizeof(internalNode);

            for (int i = 0; i < newnode.cnt; i++)
            {
                newnode.key[i] = node.key[i + node.cnt];
                newnode.child[i] = node.child[i + node.cnt];
            }

            leafNode leaf;
            internalNode internal;
            for (int i = 0; i < newnode.cnt; i++)
            {
                if (node.type == 1)
                {
                    fileRead(&leaf, newnode.child[i], 1, sizeof(leafNode));
                    leaf.parent = newnode.offset;
                    fileWrite(&leaf, newnode.child[i], 1, sizeof(leafNode));
                }
                else
                {
                    fileRead(&internal, newnode.child[i], 1, sizeof(internalNode));
                    internal.parent = newnode.offset;
                    fileWrite(&internal, newnode.child[i], 1, sizeof(internalNode));
                }
            }

            if (node.offset == info.root) //分裂到root,左右孩子指向原根节点和新分出来的结点
            {
                internalNode newroot;
                newroot.parent = 0;
                newroot.type = 0;
                newroot.offset = info.eof;
                info.eof += sizeof(internalNode);
                info.root = newroot.offset;
                newroot.cnt = 2;
                newroot.child[0] = node.offset;
                newroot.child[1] = newnode.offset;
                newroot.key[0] = node.key[0]; //最小键值
                newroot.key[1] = newnode.key[0];
                node.parent = newroot.offset;
                newnode.parent = newroot.offset;

                fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
                fileWrite(&newnode, newnode.offset, 1, sizeof(internalNode));
                fileWrite(&newroot, newroot.offset, 1, sizeof(internalNode));
            }
            else
            {
                fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
                fileWrite(&newnode, newnode.offset, 1, sizeof(internalNode));

                internalNode parent;
                fileRead(&parent, node.parent, 1, sizeof(internalNode));
                insertNode(parent, newnode.key[0], newnode.offset);
            }
        }

        void buildTree()
        {
            info.size = 0;
            info.eof = sizeof(TreeInfo);

            internalNode root;
            info.root = root.offset = info.eof;
            info.eof += sizeof(internalNode);

            leafNode leaf;
            info.head = info.tail = leaf.offset = info.eof;
            info.eof += sizeof(leafNode);

            root.parent = 0;
            root.cnt = 1;
            root.type = 1;
            root.child[0] = leaf.offset;

            leaf.parent = root.offset;
            leaf.pre = leaf.next = 0;
            leaf.cnt = 0;

            fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
            fileWrite(&root, root.offset, 1, sizeof(internalNode));
            fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
        }

        bool borrow_left(leafNode leaf)
        {
            if (leaf.pre == 0)
                return false;
            leafNode leftLeaf;
            fileRead(&leftLeaf, leaf.pre, 1, sizeof(leafNode));
            if (leaf.parent != leftLeaf.parent || leftLeaf.cnt <= Lmin)
                return false;

            Key key, newKey;
            key = leftLeaf.data[0].first;
            newKey = leftLeaf.data[1].first;
            leaf.data[leaf.cnt].first = leftLeaf.data[0].first;leaf.data[leaf.cnt].second = leftLeaf.data[0].second;
            leaf.cnt++;
            leftLeaf.cnt--;
            for (int i = 0; i < leftLeaf.cnt; i++)
                {leftLeaf.data[i].first = leftLeaf.data[i + 1].first;leftLeaf.data[i].second = leftLeaf.data[i + 1].second;}

            internalNode node;
            fileRead(&node, leaf.parent, 1, sizeof(internalNode));
            for (int i = 0; i < node.cnt; i++)
            {
                if (node.key[i] == key)
                {
                    node.key[i] = newKey;
                    break;
                }
            }

            fileWrite(&node, node.offset, 1, sizeof(internalNode));
            fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
            fileWrite(&leftLeaf, leftLeaf.offset, 1, sizeof(leafNode));
            return true;
        }

        bool borrow_right(leafNode leaf)
        {
            if (leaf.next == 0)
                return false;
            leafNode rightLeaf;
            fileRead(&rightLeaf, leaf.next, 1, sizeof(leafNode));
            if (leaf.parent != rightLeaf.parent || rightLeaf.cnt <= Lmin)
                return false;

            Key key, newKey;
            key = rightLeaf.data[0].first;
            newKey = rightLeaf.data[1].first;
            leaf.data[leaf.cnt].first = rightLeaf.data[0].first;
            leaf.data[leaf.cnt].second = rightLeaf.data[0].second;
            leaf.cnt++;
            rightLeaf.cnt--;
            for (int i = 0; i < rightLeaf.cnt; i++)
                {rightLeaf.data[i].first = rightLeaf.data[i + 1].first;rightLeaf.data[i].second = rightLeaf.data[i + 1].second;}

            internalNode node;
            fileRead(&node, leaf.parent, 1, sizeof(internalNode));
            for (int i = 0; i < node.cnt; i++)
            {
                if (node.key[i] == key)
                {
                    node.key[i] = newKey;
                    break;
                }
            }

            fileWrite(&node, node.offset, 1, sizeof(internalNode));
            fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
            fileWrite(&rightLeaf, rightLeaf.offset, 1, sizeof(leafNode));
            return true;
        }

        bool merge_left(leafNode leaf)
        {
            if (leaf.pre == 0)
                return false;
            leafNode leftLeaf;
            fileRead(&leftLeaf, leaf.pre, 1, sizeof(leafNode));
            if (leaf.parent != leftLeaf.parent)
                return false;
            for (int i = 0; i < leaf.cnt; ++i)
            {
                leftLeaf.data[leftLeaf.cnt].first = leaf.data[i].first;
                leftLeaf.data[leftLeaf.cnt].second = leaf.data[i].second;
                leftLeaf.cnt++;
            }
            leftLeaf.next = leaf.next;
            if (info.tail == leaf.offset)
            {
                info.tail = leftLeaf.offset;
                fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
            }
            else
            {
                leafNode p;
                fileRead(&p, leaf.next, 1, sizeof(leafNode));
                p.pre = leftLeaf.offset;
                fileWrite(&p, p.offset, 1, sizeof(leafNode));
            }
            fileWrite(&leftLeaf, leftLeaf.offset, 1, sizeof(leafNode));

            internalNode node;
            fileRead(&node, leaf.parent, 1, sizeof(internalNode));
            int pos = 0;
            for (; pos < node.cnt; ++pos)
            {
                if (node.key[pos] == leaf.data[0].first)
                    break;
            }
            for (int i = pos; i < node.cnt; ++i)
            {
                node.key[i] = node.key[i + 1];
                node.child[i] = node.child[i + 1];
            }
            node.cnt--;

            if (node.parent == 0 || node.cnt >= Mmin)
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
            else
                adjust_node(node);
            return true;
        }

        bool merge_right(leafNode leaf)
        {
            if (leaf.next == 0)
                return false;
            leafNode rightLeaf;
            fileRead(&rightLeaf, leaf.next, 1, sizeof(leafNode));
            if (leaf.parent != rightLeaf.parent)
                return false;
            for (int i = 0; i < rightLeaf.cnt; ++i)
            {
                leaf.data[leaf.cnt].first = rightLeaf.data[i].first;
                leaf.data[leaf.cnt].second = rightLeaf.data[i].second;
                leaf.cnt++;
            }
            leaf.next = rightLeaf.next;
            if (info.tail == rightLeaf.offset)
            {
                info.tail = leaf.offset;
                fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
            }
            else
            {
                leafNode p;
                fileRead(&p, leaf.next, 1, sizeof(leafNode));
                p.pre = leaf.offset;
                fileWrite(&p, p.offset, 1, sizeof(leafNode));
            }
            fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));

            internalNode node;
            fileRead(&node, leaf.parent, 1, sizeof(internalNode));
            int pos = 0;
            for (; pos < node.cnt; ++pos)
            {
                if (node.key[pos] == rightLeaf.data[0].first)
                    break;
            }
            for (int i = pos; i < node.cnt; ++i)
            {
                node.key[i] = node.key[i + 1];
                node.child[i] = node.child[i + 1];
            }
            node.cnt--;

            if (node.parent == 0 || node.cnt >= Mmin)
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
            else
                adjust_node(node);
            return true;
        }

        void adjust_leaf(leafNode leaf)
        {
            if (borrow_left(leaf))
                return;
            if (borrow_right(leaf))
                return;
            if (merge_left(leaf))//左右节点为Mmin或Lmin
                return;
            if (merge_right(leaf))
                return;
            fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
        }

        bool borrow_right_node(internalNode node)
        {
            if (node.parent == 0)
                return false;
            internalNode par;
            fileRead(&par, node.parent, 1, sizeof(internalNode));
            int pos = 0;
            for (; pos < par.cnt; ++pos)
                if (par.child[pos] == node.offset)
                    break;
            if (pos == par.cnt - 1)
                return false;
            internalNode right;
            fileRead(&right, par.child[pos + 1], 1, sizeof(internalNode));
            if (right.cnt <= Mmin)
                return false;

            node.key[node.cnt] = right.key[0];
            node.child[node.cnt] = right.child[0];
            ++node.cnt;
            --right.cnt;
            for (int i = 0; i < right.cnt; ++i)
                right.key[i] = right.key[i + 1];
            for (int i = 0; i < right.cnt; ++i)
                right.child[i] = right.child[i + 1];
            par.key[pos + 1] = right.key[0];

            if (node.type == 1)
            {
                leafNode son;
                fileRead(&son, node.child[node.cnt - 1], 1, sizeof(leafNode));
                son.parent = node.offset;
                fileWrite(&son, son.offset, 1, sizeof(leafNode));
            }
            else
            {
                internalNode son;
                fileRead(&son, node.child[node.cnt - 1], 1, sizeof(internalNode));
                son.parent = node.offset;
                fileWrite(&son, son.offset, 1, sizeof(internalNode));
            }

            fileWrite(&node, node.offset, 1, sizeof(internalNode));
            fileWrite(&right, right.offset, 1, sizeof(internalNode));
            fileWrite(&par, par.offset, 1, sizeof(internalNode));
            return true;
        }

        bool borrow_left_node(internalNode node)
        {
            if (node.parent == 0)
                return false;
            internalNode par;
            fileRead(&par, node.parent, 1, sizeof(internalNode));
            int pos = 0;
            for (; pos < par.cnt; ++pos)
                if (par.child[pos] == node.offset)
                    break;
            if (pos == 0)
                return false;
            internalNode left;
            fileRead(&left, par.child[pos - 1], 1, sizeof(internalNode));
            if (left.cnt <= Mmin)
                return false;

            --left.cnt;
            for (int i = node.cnt - 1; i >= 0; --i)
            {
                node.key[i + 1] = node.key[i];
                node.child[i + 1] = node.child[i];
            }
            node.key[0] = left.key[left.cnt];
            node.child[0] = left.child[left.cnt];
            ++node.cnt;
            par.key[pos] = node.key[0];

            if (node.type == 1)
            {
                leafNode son;
                fileRead(&son, node.child[0], 1, sizeof(leafNode));
                son.parent = node.offset;
                fileWrite(&son, son.offset, 1, sizeof(leafNode));
            }
            else
            {
                internalNode son;
                fileRead(&son, node.child[0], 1, sizeof(internalNode));
                son.parent = node.offset;
                fileWrite(&son, son.offset, 1, sizeof(internalNode));
            }

            fileWrite(&node, node.offset, 1, sizeof(internalNode));
            fileWrite(&left, left.offset, 1, sizeof(internalNode));
            fileWrite(&par, par.offset, 1, sizeof(internalNode));
            return true;
        }

        bool merge_right_node(internalNode node)
        {
            if (node.parent == 0)
                return false;
            internalNode par;
            fileRead(&par, node.parent, 1, sizeof(internalNode));
            int pos = 0;
            for (; pos < par.cnt; ++pos)
                if (par.child[pos] == node.offset)
                    break;
            if (pos == par.cnt - 1)
                return false;
            internalNode right;
            fileRead(&right, par.child[pos + 1], 1, sizeof(internalNode));

            //修改node及child
            for (int i = 0; i < right.cnt; ++i)
            {
                node.key[node.cnt] = right.key[i];
                node.child[node.cnt] = right.child[i];
                if (node.type == 1)
                {
                    leafNode son;
                    fileRead(&son, right.child[i], 1, sizeof(leafNode));
                    son.parent = node.offset;
                    fileWrite(&son, son.offset, 1, sizeof(leafNode));
                }
                else
                {
                    internalNode son;
                    fileRead(&son, right.child[i], 1, sizeof(internalNode));
                    son.parent = node.offset;
                    fileWrite(&son, son.offset, 1, sizeof(internalNode));
                }
                ++node.cnt;
            }
            fileWrite(&node, node.offset, 1, sizeof(internalNode));

            //修改parent
            --par.cnt;
            for (int i = pos + 1; i < par.cnt; ++i)
                par.key[i] = par.key[i + 1], par.child[i] = par.child[i + 1];
            if (par.parent == 0 || par.cnt >= Mmin)
                fileWrite(&par, par.offset, 1, sizeof(internalNode));
            else
                adjust_node(par);
            return true;
        }

        bool merge_left_node(internalNode node)
        {
            if (node.parent == 0)
                return false;
            internalNode par;
            fileRead(&par, node.parent, 1, sizeof(internalNode));
            int pos = 0;
            for (; pos < par.cnt; ++pos)
                if (par.child[pos] == node.offset)
                    break;
            if (pos == 0)
                return false;
            internalNode left;
            fileRead(&left, par.child[pos - 1], 1, sizeof(internalNode));

            //修改leftnode及child
            for (int i = 0; i < node.cnt; ++i)
            {
                left.key[left.cnt] = node.key[i];
                left.child[left.cnt] = node.child[i];
                if (left.type == 1)
                {
                    leafNode son;
                    fileRead(&son, node.child[i], 1, sizeof(leafNode));
                    son.parent = left.offset;
                    fileWrite(&son, son.offset, 1, sizeof(leafNode));
                }
                else
                {
                    internalNode son;
                    fileRead(&son, node.child[i], 1, sizeof(internalNode));
                    son.parent = left.offset;
                    fileWrite(&son, son.offset, 1, sizeof(internalNode));
                }
                ++left.cnt;
            }

            for (int i = pos; i < par.cnt - 1; ++i)
            {
                par.key[i] = par.key[i + 1];
                par.child[i] = par.child[i + 1];
            }
            --par.cnt;
            fileWrite(&left, left.offset, 1, sizeof(internalNode));
            if (par.parent == 0 || par.cnt >= Mmin)
                fileWrite(&par, par.offset, 1, sizeof(internalNode));
            else
                adjust_node(par);
            return true;
        }

        void adjust_node(internalNode node)
        {
            if (borrow_left_node(node))
                return;
            if (borrow_right_node(node))
                return;
            if (merge_left_node(node))
                return;
            if (merge_right_node(node))
                return;
        }

    public:
        BTree()
        {
            filename.naming("abc");
            file = nullptr;
            openFile();
            if (!fileExist)
            {
                buildTree();
            }
        }

        BTree(const char *fname)
        {
            filefromName.naming(fname);
            filefrom = fopen(filefromName.str, "rb+");
            TreeInfo newinfo;
            copy_fileRead(&newinfo, info_offset, 1, sizeof(TreeInfo));

            prevLeaf = 0;
            info.size = newinfo.size;
            info.root = info.eof = sizeof(TreeInfo);

            fileWrite(info, info_offset, 1, sizeof(TreeInfo));
            copy_node(info.root, newinfo.root, 0);
            fileWrite(info, info_offset, 1, sizeof(TreeInfo));
            fclose(filefrom);
        }

        ~BTree()
        {
            closeFile();
        }

        // Clear the BTree
        void clear()
        {
            file = fopen(filename.str, "wb+");
            buildTree(); //重新建树，把原来覆盖掉
        }

        bool insert(const Key &key, const Value &value)
        {
            off_t leaf_offset = locate_leaf(key, info.root);
            leafNode leaf;

            if (info.size == 0 || leaf_offset == 0) //空树
            {
                fileRead(&leaf, info.head, 1, sizeof(leafNode));
                bool ret = insertLeaf(leaf, key, value);
                if (ret == false)
                    return ret;

                off_t offset = leaf.parent;
                internalNode node;
                while (offset != 0) //向上设置第一个点的值为新的key直到设置到根节点
                {
                    fileRead(&node, offset, 1, sizeof(internalNode));
                    node.key[0] = key;
                    fileWrite(&node, offset, 1, sizeof(internalNode));
                    offset = node.parent;
                }
                return ret;
            }
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            bool ret = insertLeaf(leaf, key, value);
            return ret;
        }

        bool modify(const Key &key, const Value &value)
        {
            iterator it = find(key);
            leafNode p;
            it.tree->fileRead(&p, it.offset, 1, sizeof(leafNode));
            p.data[it.place].second = value;
            it.tree->fileWrite(&p, it.offset, 1, sizeof(leafNode));
            return true;
        }

        Value at(const Key &key)
        {
            iterator it = find(key);
            if (it == end())
                return Value();
            leafNode leaf;
            fileRead(&leaf, it.offset, 1, sizeof(leafNode));
            return leaf.data[it.place].second;
        }

        bool erase(const Key &key)
        {
            off_t leaf_offset = locate_leaf(key, info.root);
            if (leaf_offset == 0)
                return false;
            leafNode leaf;
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            int pos = 0;
            for (; pos < leaf.cnt; ++pos)
            {
                if (leaf.data[pos].first == key)
                    break;
            }
            if (pos == leaf.cnt)
                return false;
            for (int i = pos + 1; i < leaf.cnt; ++i)
            {
                leaf.data[i - 1].first = leaf.data[i].first;
                leaf.data[i - 1].second = leaf.data[i].second;
            }
            leaf.cnt--;
            off_t node_offset = leaf.parent;
            internalNode node;
            while (pos == 0)
            {
                if (node_offset == 0)
                    break;
                fileRead(&node, node_offset, 1, sizeof(internalNode));
                pos = 0;
                for (; pos < node.cnt; pos++)
                {
                    if (node.key[pos] == key)
                        break;
                }
                node.key[pos] = leaf.data[0].first;
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
                node_offset = node.parent;
            }
            info.size--;
            fileWrite(&info, info_offset, 1, sizeof(TreeInfo));
            if (leaf.cnt < Lmin)
            {
                adjust_leaf(leaf);
                return true;
            }
            fileWrite(&leaf, leaf_offset, 1, sizeof(leafNode));
            return true;
        }

        class iterator
        {
            friend class BTree;

        private:
            // Your private members go here
            off_t offset; //offset:迭代器指向的元素所在的叶节点的开头文件中位置
            int place;    // place：在节点中的第几个（从0开始）
            BTree *tree;

        public:
            iterator()
            {
                offset = 0;
                place = 0;
                tree = nullptr;
            }
            iterator(BTree *from, off_t tempoffset = 0, int x = 0)
            {
                tree = from;
                offset = tempoffset;
                place = x;
            }

            iterator(const iterator &other)
            {
                offset = other.offset;
                place = other.place;
                tree = other.tree;
            }

            // modify by iterator
            bool modify(const Value &value)
            {
                leafNode p;
                tree->fileRead(&p, offset, 1, sizeof(leafNode));
                if (place >= p.cnt)
                {
                    return false;
                }
                p.data[place].second = value;
                tree->fileWrite(&p, offset, 1, sizeof(leafNode));
                return true;
            }

            Key getKey() const
            {
                leafNode p;
                tree->fileRead(&p, offset, 1, sizeof(leafNode));
                return p.data[place].first;
            }

            Value getValue() const
            {
                leafNode p;
                tree->fileRead(&p, offset, 1, sizeof(leafNode));
                return p.data[place].second;
            }

            iterator operator++(int)
            {
                iterator ret = *this;
                ++(*this);
                return ret;
            }

            iterator &operator++()
            {
                if (*this == tree->end())
                {
                    tree = nullptr;
                    offset = 0;
                    place = 0;
                    return *this;
                }

                leafNode p;
                tree->fileRead(&p, offset, 1, sizeof(leafNode));
                if (place == p.cnt - 1)
                {
                    if (p.next == 0)
                        place++; //end()
                    else
                    {
                        offset = p.next;
                        place = 0;
                    }
                }
                else
                    place++;
                return *this;
            }
            iterator operator--(int)
            {
                iterator ret = *this;
                --(*this);
                return ret;
            }

            iterator &operator--()
            {
                if (*this == tree->begin())
                {
                    tree = nullptr;
                    offset = 0;
                    place = 0;
                    return *this;
                }

                leafNode p, q;
                tree->fileRead(&p, offset, 1, sizeof(leafNode));
                if (place == 0)
                {
                    offset = p.pre;
                    tree->fileRead(&q, p.pre, 1, sizeof(leafNode));
                    place = q.cnt - 1;
                }
                else
                    place--;
                return *this;
            }

            // Overloaded of operator '==' and '!='
            // Check whether the iterators are same
            bool operator==(const iterator &rhs) const
            {
                return (tree == rhs.tree && offset == rhs.offset && place == rhs.place);
            }

            bool operator!=(const iterator &rhs) const
            {
                return (tree != rhs.tree || offset != rhs.offset || place != rhs.place);
            }
        };

        iterator begin()
        {
            return iterator(this, info.head, 0);
        }

        // return an iterator to the end(the next element after the last)
        iterator end()
        {
            leafNode tail;
            fileRead(&tail, info.tail, 1, sizeof(leafNode));
            return iterator(this, info.tail, tail.cnt);
        }

        iterator find(const Key &key)
        {
            off_t leaf_offset = locate_leaf(key, info.root);
            if (leaf_offset == 0)
                return end();

            leafNode leaf;
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            for (int i = 0; i < leaf.cnt; i++)
                if (leaf.data[i].first == key)
                    return iterator(this, leaf_offset, i);
            return end();
        }

        // return an iterator whose key is the smallest key greater or equal than 'key'
        iterator lower_bound(const Key &key)
        {
            iterator temp = find(key);
            return --temp;
        }
    };
}; // namespace sjtu
