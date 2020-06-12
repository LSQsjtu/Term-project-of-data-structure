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

    private:
        // Your private members go here
        static const int M = (4096 - sizeof(bool) - sizeof(int) * 3) / (sizeof(int) + sizeof(Key));
        static const int L = (4096 - sizeof(int) * 5) / (sizeof(Key) + sizeof(Value));
        static const int Mmin = M / 2;
        static const int Lmin = L / 2;
        struct name
        {
            char *str;
            name() { str = new char[10]; }
            ~name()
            {
                if (str != nullptr)
                    delete str;
            }
            void naming(const char *oth) { strcpy(str, oth); }
        };

        struct value_type
        {
            Key key;
            Value val;
        };

        struct TreeInfo
        {
            int head = 0;
            int tail = 0;
            int root = 0; //root of BTree
            int size = 0; //size of BTree
            int eof = 0;  //end of file
        };

        struct leafNode
        {
            int offset = 0;
            int parent = 0;
            int pre;
            int next;
            int cnt = 0; //number of pairs in leaf
            value_type data[L];
        };

        struct internalNode
        {
            int offset;
            int parent;
            int child[M];
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

        //place指针，设为void，可以写入任意类型;offset：在文件中读取的位置;num:向后读多少个
        void fileRead(void *place, int offset, int num, int size) const
        {
            fseek(file, offset, SEEK_SET);
            fread(place, size, num, file);
        }

        void fileWrite(void *place, int offset, int num, int size) const
        {
            fseek(file, offset, SEEK_SET);
            fwrite(place, size, num, file);
        }

        FILE *filefrom;
        name filefromName;

        void copy_fileRead(void *place, int offset, int num, int size) const
        {
            fseek(filefrom, offset, SEEK_SET);
            fread(place, size, num, filefrom);
        }

        int prevLeaf;                                                  // 前一个叶子
        void copy_leaf(int offset, int from_offset, int parent_offset) //在复制函数中会用到位置，from叶子的位置，from叶子的父亲的地址
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
                leaf.data[i] = leafFrom.data[i];
            }

            fileWrite(&leaf, offset, 1, sizeof(leafNode));

            info.eof += sizeof(leafNode);
            prevLeaf = offset; //为下一个叶子的连接做准备
        }

        void copy_node(int offset, int from_offset, int parent_offset)
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

        int locate_leaf(const Key &key, int offset) const //返回leaf.offset，从root开始找，key[0]为最小值
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
            int pos = 0;

            for (; pos < leaf.cnt; ++pos) //找到key位于pos - 1到pos之间
            {
                if (key == leaf.data[pos].key)
                    return false; //已经存在
                if (key < leaf.data[pos].key)
                    break;
            }

            for (int i = leaf.cnt - 1; i >= pos; --i)
            {
                leaf.data[i + 1].key = leaf.data[i].key;
                leaf.data[i + 1].val = leaf.data[i].val;
            }
            leaf.data[pos].key = key;
            leaf.data[pos].val = value;

            ++leaf.cnt;
            ++info.size;

            fileWrite(&info, 0, 1, sizeof(TreeInfo));
            if (leaf.cnt < L)
                fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
            else
                splitLeaf(leaf, key);
            return true;
        }

        void insertNode(internalNode &node, const Key &key, int ch)
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

        void splitLeaf(leafNode &leaf, const Key &key)
        {
            leafNode newleaf;
            newleaf.cnt = leaf.cnt - (leaf.cnt / 2);
            leaf.cnt = leaf.cnt / 2;
            newleaf.parent = leaf.parent;
            newleaf.offset = info.eof; //尾部增加叶子
            info.eof += sizeof(leafNode);

            for (int i = 0; i < newleaf.cnt; i++)
            {
                newleaf.data[i] = leaf.data[i + leaf.cnt];
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

            fileWrite(&info, 0, 1, sizeof(TreeInfo));
            fileWrite(&leaf, leaf.offset, 1, sizeof(leafNode));
            fileWrite(&newleaf, newleaf.offset, 1, sizeof(leafNode));

            internalNode parent;
            fileRead(&parent, leaf.parent, 1, sizeof(internalNode));
            insertNode(parent, newleaf.data[0].key, newleaf.offset);
        }

        void splitNode(internalNode &node)
        {
            internalNode newnode;
            newnode.cnt = node.cnt - (node.cnt / 2);
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

                fileWrite(&info, 0, 1, sizeof(TreeInfo));
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
                fileWrite(&newnode, newnode.offset, 1, sizeof(internalNode));
                fileWrite(&newroot, newroot.offset, 1, sizeof(internalNode));
            }
            else
            {
                fileWrite(&info, 0, 1, sizeof(TreeInfo));
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
            info.eof += sizeof(leafNode);

            root.parent = 0;
            root.cnt = 1;
            root.type = 1;
            root.child[0] = leaf.offset;

            leaf.parent = root.offset;
            leaf.pre = leaf.next = 0;
            leaf.cnt = 0;

            fileWrite(&info, 0, 1, sizeof(TreeInfo));
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
            key = leaf.data[0].key;
            newKey = leftLeaf.data[leftLeaf.cnt - 1].key;
            leaf.data[leaf.cnt] = leftLeaf.data[0];
            for (int i = leaf.cnt - 1; i >= 0; --i)
                leaf.data[i + 1] = leaf.data[i];
            leaf.cnt++;
            leftLeaf.cnt--;
            leaf.data[0] = leftLeaf.data[leftLeaf.cnt];

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
            key = rightLeaf.data[0].key;
            newKey = rightLeaf.data[1].key;
            leaf.data[leaf.cnt] = rightLeaf.data[0];
            leaf.cnt++;
            rightLeaf.cnt--;
            for (int i = 0; i < rightLeaf.cnt; i++)
            {
                rightLeaf.data[i] = rightLeaf.data[i + 1];
            }

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
                leftLeaf.data[leftLeaf.cnt] = leaf.data[i];
                leftLeaf.cnt++;
            }
            leftLeaf.next = leaf.next;
            if (info.tail == leaf.offset)
            {
                info.tail = leftLeaf.offset;
                fileWrite(&info, 0, 1, sizeof(TreeInfo));
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
                if (node.key[pos] == leaf.data[0].key)
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
                leaf.data[leaf.cnt] = rightLeaf.data[i];
                leaf.cnt++;
            }
            leaf.next = rightLeaf.next;
            if (info.tail == rightLeaf.offset)
            {
                info.tail = leaf.offset;
                fileWrite(&info, 0, 1, sizeof(TreeInfo));
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
                if (node.key[pos] == rightLeaf.data[0].key)
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
            if (merge_left(leaf)) //左右节点为Mmin或Lmin
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
            {
                right.key[i] = right.key[i + 1];
                right.child[i] = right.child[i + 1];
            }
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

            --par.cnt;
            for (int i = pos; i < par.cnt; ++i)
            {
                par.key[i] = par.key[i + 1];
                par.child[i] = par.child[i + 1];
            }
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
            internalNode par; //node非头节点且无左右
            fileRead(&par, node.parent, 1, sizeof(internalNode));
            if (par.parent == 0)
            {
                info.root = node.offset;
                node.parent = 0;
                fileWrite(&info, 0, 1, sizeof(TreeInfo));
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
            }
        }

    public:
        BTree()
        {
            filename.naming("abc");
            file = fopen(filename.str, "rb+");
            if (!file)
            {
                file = fopen(filename.str, "wb+");
                buildTree();
            }
            else
                fileRead(&info, 0, 1, sizeof(TreeInfo));
        }

        BTree(const char *fname)
        {
            filefromName.naming(fname);
            filefrom = fopen(filefromName.str, "rb+");
            TreeInfo newinfo;
            copy_fileRead(&newinfo, 0, 1, sizeof(TreeInfo));

            prevLeaf = 0;
            info.size = newinfo.size;
            info.root = info.eof = sizeof(TreeInfo);

            fileWrite(info, 0, 1, sizeof(TreeInfo));
            copy_node(info.root, newinfo.root, 0);
            fileWrite(info, 0, 1, sizeof(TreeInfo));
            fclose(filefrom);
        }

        ~BTree()
        {
            fclose(file);
        }

        // Clear the BTree
        void clear()
        {
            file = fopen(filename.str, "wb+");
            buildTree(); //重新建树，把原来覆盖掉
        }

        bool insert(const Key &key, const Value &value)
        {
            int leaf_offset = locate_leaf(key, info.root);
            leafNode leaf;

            if (info.size == 0 || leaf_offset == 0) //空树
            {
                fileRead(&leaf, info.head, 1, sizeof(leafNode));
                bool ret = insertLeaf(leaf, key, value);
                if (ret == false)
                    return ret;

                int offset = leaf.parent;
                internalNode node;
                while (offset != 0) //向上设置第一个点的值为新的key直到设置到根节点
                {
                    fileRead(&node, offset, 1, sizeof(internalNode));
                    node.key[0] = key;
                    fileWrite(&node, offset, 1, sizeof(internalNode));
                    offset = node.parent;
                }
                return true;
            }
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            bool ret = insertLeaf(leaf, key, value);
            return ret;
        }

        bool modify(const Key &key, const Value &value)
        {
            int leaf_offset = locate_leaf(key, info.root);
            if (leaf_offset == 0)
                return end();

            leafNode leaf;
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            for (int i = 0; i < leaf.cnt; i++)
                if (leaf.data[i].key == key)
                {
                    leaf.data[i].val = value;
                    fileWrite(&leaf, leaf_offset, 1, sizeof(leafNode));
                    return true;
                }
        }

        Value at(const Key &key)
        {
            int leaf_offset = locate_leaf(key, info.root);
            if (leaf_offset == 0)
                return Value();

            leafNode leaf;
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            for (int i = 0; i < leaf.cnt; i++)
                if (leaf.data[i].key == key)
                    return leaf.data[i].val;
            return Value();
        }

        bool erase(const Key &key)
        {
            int leaf_offset = locate_leaf(key, info.root);
            if (leaf_offset == 0)
                return false;
            leafNode leaf;
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            int pos = 0;
            for (; pos < leaf.cnt; ++pos)
            {
                if (leaf.data[pos].key == key)
                    break;
            }
            if (pos == leaf.cnt)
                return false;
            for (int i = pos + 1; i < leaf.cnt; ++i)
                leaf.data[i - 1] = leaf.data[i];
            leaf.cnt--;
            int node_offset = leaf.parent;
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
                node.key[pos] = leaf.data[0].key;
                fileWrite(&node, node.offset, 1, sizeof(internalNode));
                node_offset = node.parent;
            }
            info.size--;
            fileWrite(&info, 0, 1, sizeof(TreeInfo));
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
            int offset; //offset:迭代器指向的元素所在的叶节点的开头文件中位置
            int place;  // place：在节点中的第几个（从0开始）
            BTree *tree;

        public:
            iterator()
            {
                offset = 0;
                place = 0;
                tree = nullptr;
            }
            iterator(BTree *from, int tempoffset = 0, int x = 0)
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
                p.data[place].val = value;
                tree->fileWrite(&p, offset, 1, sizeof(leafNode));
                return true;
            }

            Key getKey() const
            {
                leafNode p;
                tree->fileRead(&p, offset, 1, sizeof(leafNode));
                return p.data[place].key;
            }

            Value getValue() const
            {
                leafNode p;
                tree->fileRead(&p, offset, 1, sizeof(leafNode));
                return p.data[place].val;
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
            int leaf_offset = locate_leaf(key, info.root);
            if (leaf_offset == 0)
                return end();

            leafNode leaf;
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            for (int i = 0; i < leaf.cnt; i++)
                if (leaf.data[i].key == key)
                    return iterator(this, leaf_offset, i);
            return end();
        }

        // return an iterator whose key is the smallest key greater or equal than 'key'
        iterator lower_bound(const Key &key)
        {
            internalNode p;
            int offset = info.root;
            fileRead(&p, offset, 1, sizeof(internalNode));

            while (p.type == 0)
            {
                int pos;
                for (pos = 0; pos < p.cnt; pos++)
                    if (key < p.key[pos])
                        break;
                if (pos == 0)
                    offset = p.child[0];
                else
                    offset = p.child[pos - 1];
                fileRead(&p, offset, 1, sizeof(internalNode));
            }
            int pos = 0;
            int leaf_offset;
            for (; pos < p.cnt; pos++)
                if (key < p.key[pos])
                    break;
            if (pos == 0)
                leaf_offset = p.child[0];
            else
                leaf_offset = p.child[pos - 1];

            leafNode leaf;
            fileRead(&leaf, leaf_offset, 1, sizeof(leafNode));
            for (int i = 0; i < leaf.cnt; i++)
                if (leaf.data[i].key >= key)
                    return iterator(this, leaf_offset, i);
            return iterator(this, leaf.next, 0);
        }
    };
} // namespace sjtu
