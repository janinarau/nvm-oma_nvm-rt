#include "RBTree.h"
#include <system/memory/PageBalancer.h>
#include <system/service/logger.h>

template class RBTree<phys_page_handle>;

template class Node_Comparator<phys_page_handle>;
template <>
int Node_Comparator<phys_page_handle>::compare(phys_page_handle node1,
                                               phys_page_handle node2) {
    if (node2.access_count != node1.access_count) {
        return node1.access_count - node2.access_count;
    }
    return node1.phys_address -
           node2.phys_address;  // Just to keep an absolute ordner in the tree
}
template <>
bool Node_Comparator<phys_page_handle>::equals(phys_page_handle node1,
                                               phys_page_handle node2) {
    return node1.access_count == node2.access_count &&
           node1.phys_address == node2.phys_address;
}

template class RBTree<unsigned int>;
template class Node_Comparator<unsigned int>;
template <>
int Node_Comparator<unsigned int>::compare(unsigned int node1,
                                           unsigned int node2) {
    return node1 - node2;
}
template <>
bool Node_Comparator<unsigned int>::equals(unsigned int node1,
                                           unsigned int node2) {
    return node1 == node2;
}

template <class T>
RBTree<T>::RBTree() {
    root = 0;
    element_count = 0;
}

template <class T>
unsigned long RBTree<T>::fast_log2(unsigned long n) {
    static const int table[64] = {
        0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54, 33, 42, 3,  61,
        51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4,  62,
        57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21, 56,
        45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5,  63};

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;

    return table[(n * 0x03f6eaf2cd271461) >> 58];
}

template <class T>
void RBTree<T>::insert(struct RBTree<T>::node *element) {
    // Maximum height
    unsigned long max_height = 2 * fast_log2(element_count + 2) - 2;
    max_height++;
    // List of parent pointers, on the way to the target
    struct RBTree<T>::node *parents[max_height];
    struct RBTree<T>::node **pParent = 0;

    // search for the place to insert
    unsigned int pidx = 0;
    struct RBTree<T>::node *act = root;
    while (act != 0) {
        parents[pidx++] = act;
        pParent = &parents[pidx - 1];
        if (Node_Comparator<T>::compare(element->value, act->value) > 0) {
            act = act->right;
        } else {
            act = act->left;
        }
        if (pidx > max_height) {
            log_error("Size overflowed");
            while (1)
                ;
        }
    }

    element->color = RED;
    element->right = 0;
    element->left = 0;

    element_count++;
    // If the root was empty, pParent is 0
    if (pParent >= &parents[0]) {
        // Node to insert the new node at
        struct RBTree<T>::node *p = *pParent;
        if (Node_Comparator<T>::compare(element->value, p->value) > 0) {
            p->right = element;
        } else {
            p->left = element;
        }

        /**
         * Now rebalance the tree, distinguish between 6 cases
         */
        do {
            p = *pParent;
            // Case 1: Parent node was black, no need to rebalance
            if (p->color == BLACK) {
                return;
            }
            // Case 2: Parent is not black, but the root
            if (--pParent < &parents[0]) {
                p->color = BLACK;
                return;
            }
            // Case 3: Parent is red and not the root, has none or a black uncle
            // and is inner child
            struct RBTree<T>::node *g = *pParent;
            bool pDirection = p == g->left;
            struct RBTree<T>::node *u = pDirection ? g->right : g->left;
            if (u == 0 || u->color == BLACK) {
                if (element != (pDirection ? p->left : p->right)) {
                    (pDirection ? p->right : p->left) =
                        (pDirection ? element->left : element->right);
                    (pDirection ? element->left : element->right) = p;
                    (pDirection ? g->left : g->right) = element;
                    u = p;
                    p = element;
                    element = u;
                }
                // Case 4: ... is outer child
                (pDirection ? g->left : g->right) =
                    (pDirection ? p->right : p->left);
                (pDirection ? p->right : p->left) = g;
                p->color = BLACK;
                g->color = RED;
                if (--pParent < &parents[0]) {
                    root = p;
                } else {
                    element = *pParent;
                    (g == element->left ? element->left : element->right) = p;
                }
                return;
            }
            // Case 5: Uncle and Parent are red
            p->color = BLACK;
            u->color = BLACK;
            g->color = RED;
            element = g;
        } while (--pParent >= &parents[0]);
    }
    // Replace root
    root = element;
}

template <class T>
void RBTree<T>::remove(struct RBTree<T>::node *element) {
    // Find the searched node
    // Maximum height
    unsigned long max_height = 2 * fast_log2(element_count + 2) - 2;
    max_height++;
    // List of parent pointers, on the way to the target
    struct RBTree<T>::node *parents[max_height];
    struct RBTree<T>::node **pParent = 0;

    // search for the matching node
    unsigned int pidx = 0;
    struct RBTree<T>::node *act = root;
    while (act != 0) {
        parents[pidx++] = act;
        pParent = &parents[pidx - 1];
        if (Node_Comparator<T>::equals(act->value, element->value)) {
            break;
        } else {
            if (Node_Comparator<T>::compare(element->value, act->value) > 0) {
                act = act->right;
            } else {
                act = act->left;
            }
        }
    }

    if (pidx == 0) {
        return;
    }

    struct RBTree<T>::node *r = parents[pidx - 1];

    if (!Node_Comparator<T>::equals(r->value, element->value)) {
        return;
    }

    /**
     * Node has 2 childs, swap the content with the minimum of the right subtree
     * (will not destroy the ordering (cause the node will be deleted) nor the
     * rb balance, cause only the content will be swapped)
     */
    if (r->right != 0 && r->left != 0) {
        act = r->right;
        while (act != 0) {
            parents[pidx++] = act;
            pParent = &parents[pidx - 1];
            act = act->left;
        }

        struct RBTree<T>::node *swap = parents[pidx - 1];
        r->value = swap->value;
        r = swap;
    }

    element_count--;
    pParent--;

    // Simple cases: r is red (has no child then)
    if (r->color == RED) {
        // Check if it was the root
        if (pParent < &parents[0]) {
            root = 0;
        } else {
            ((*pParent)->left == r ? (*pParent)->left : (*pParent)->right) = 0;
        }
    }
    // r is black, and has a red child
    else if (r->right != 0 || r->left != 0) {
        bool pDirection = (*pParent)->left == r;
        // log("Deleting a black node with a " <<
        // (r->right->color==RED?"RED":"BLACK") << " right child");
        // might be the root
        if (pParent < &parents[0]) {
            root = r->right != 0 ? r->right : r->left;
        } else {
            (pDirection ? (*pParent)->left : (*pParent)->right) =
                (r->right != 0 ? r->right : r->left);
        }
        (r->right != 0 ? r->right : r->left)->color = BLACK;
    }
    // r is black, has no child
    else {
        struct RBTree<T>::node *n = 0;
        struct RBTree<T>::node *p, *s, *c;
        bool pDirection;

        // Not delete root
        if (pParent >= &parents[0]) {
            p = *pParent;
            pDirection = p->left == r;
            (pDirection ? p->left : p->right) = 0;
            s = (pDirection ? p->right : p->left);
            // log("Before s check");
            // log("Node " << found_node << " had sibling " << s->value);
            if (s->color == RED) {
                // log("L2");
                goto del_L2;
                // log("DL2");
            }
            if ((pDirection ? s->right : s->left) != 0) {
                goto del_L5;
            }
            if ((pDirection ? s->left : s->right) != 0) {
                goto del_L4;
            }
            if (p->color == RED) {
                goto del_L3;
            }

            s->color = RED;
            n = p;
            while (--pParent >= &parents[0]) {
                p = *pParent;
                pDirection = n == p->left;
                s = (pDirection ? p->right : p->left);
                if (s->color == RED) {
                    goto del_L2;
                }
                if ((pDirection ? s->right : s->left)->color == RED) {
                    goto del_L5;
                }
                if ((pDirection ? s->left : s->right)->color == RED) {
                    goto del_L4;
                }
                if (p->color == RED) {
                    goto del_L3;
                }

                s->color = RED;
                n = p;
            }
        }

        root = n;
        return;

    del_L2:
        s->color = BLACK;
        p->color = RED;
        c = (pDirection ? s->left : s->right);
        // log("Siblings " << (pDirection?"left":"right") << " child is " << c);
        (pDirection ? p->right : p->left) = c;
        (pDirection ? s->left : s->right) = p;
        if (--pParent < &parents[0]) {
            root = s;
        } else {
            struct RBTree<T>::node *k = *pParent;
            bool pDirection = p == k->left;
            (pDirection ? k->left : k->right) = s;
            // Fix parent list
            pParent++;
            *pParent = s;
            pParent++;
        }
        s = c;
        if ((pDirection ? s->right : s->left) != 0 &&
            (pDirection ? s->right : s->left)->color == RED) {
            goto del_L5;
        }
        if ((pDirection ? s->left : s->right) != 0 &&
            (pDirection ? s->left : s->right)->color == RED) {
            goto del_L4;
        }

    del_L3:
        s->color = RED;
        p->color = BLACK;
        return;

    del_L4:
        c = (pDirection ? s->left : s->right);
        c->color = BLACK;
        (pDirection ? s->left : s->right) = (pDirection ? c->right : c->left);
        (pDirection ? c->right : c->left) = s;
        (pDirection ? p->right : p->left) = c;
        s->color = RED;
        s = c;

    del_L5:
        s->color = p->color;
        p->color = BLACK;
        (pDirection ? p->right : p->left) = (pDirection ? s->left : s->right);
        (pDirection ? s->left : s->right) = p;
        ((pDirection ? s->right : s->left))->color = BLACK;
        if (--pParent < &parents[0]) {
            root = s;
        } else {
            n = *pParent;
            bool pDirection = p == n->left;
            (pDirection ? n->left : n->right) = s;
        }

        return;
    }

    return;
}

template <class T>
T RBTree<T>::pop_minimum() {
    // Find the smallest node first
    // Maximum height
    unsigned long max_height = 2 * fast_log2(element_count + 2) - 2;
    max_height++;
    // List of parent pointers, on the way to the target
    struct RBTree<T>::node *parents[max_height];
    struct RBTree<T>::node **pParent = 0;

    // search for the smallest node
    unsigned int pidx = 0;
    struct RBTree<T>::node *act = root;
    while (act != 0) {
        parents[pidx++] = act;
        pParent = &parents[pidx - 1];
        act = act->left;
    }
    struct RBTree<T>::node *r = parents[pidx - 1];
    T found_node = r->value;
    element_count--;
    pParent--;

    // Simple cases: r is red (has no child then)
    if (r->color == RED) {
        // Check if it was the root
        if (pParent < &parents[0]) {
            root = 0;
        } else {
            (*pParent)->left = 0;
        }
    }
    // r is black, and has a red child
    else if (r->right != 0) {
        // log("Deleting a black node with a " <<
        // (r->right->color==RED?"RED":"BLACK") << " right child");
        // might be the root
        if (pParent < &parents[0]) {
            root = r->right;
        } else {
            (*pParent)->left = r->right;
        }
        r->right->color = BLACK;
    }
    // r is black, has no child
    else {
        struct RBTree<T>::node *n = 0;
        struct RBTree<T>::node *p, *s, *c;
        bool pDirection;

        // Not delete root
        if (pParent >= &parents[0]) {
            p = *pParent;
            pDirection = true;
            p->left = 0;
            s = p->right;
            // log("Before s check");
            // log("Node " << found_node << " had sibling " << s->value);
            if (s->color == RED) {
                // log("L2");
                goto del_L2;
                // log("DL2");
            }
            if (s->right != 0) {
                goto del_L5;
            }
            if (s->left != 0) {
                goto del_L4;
            }
            if (p->color == RED) {
                goto del_L3;
            }

            s->color = RED;
            n = p;
            while (--pParent >= &parents[0]) {
                p = *pParent;
                pDirection = n == p->left;
                s = (pDirection ? p->right : p->left);
                if (s->color == RED) {
                    goto del_L2;
                }
                if ((pDirection ? s->right : s->left)->color == RED) {
                    goto del_L5;
                }
                if ((pDirection ? s->left : s->right)->color == RED) {
                    goto del_L4;
                }
                if (p->color == RED) {
                    goto del_L3;
                }

                s->color = RED;
                n = p;
            }
        }

        root = n;
        return found_node;

    del_L2:
        s->color = BLACK;
        p->color = RED;
        c = (pDirection ? s->left : s->right);
        // log("Siblings " << (pDirection?"left":"right") << " child is " << c);
        (pDirection ? p->right : p->left) = c;
        (pDirection ? s->left : s->right) = p;
        if (--pParent < &parents[0]) {
            root = s;
        } else {
            struct RBTree<T>::node *k = *pParent;
            bool pDirection = p == k->left;
            (pDirection ? k->left : k->right) = s;
            // Fix parent list
            pParent++;
            *pParent = s;
            pParent++;
        }
        s = c;
        if ((pDirection ? s->right : s->left) != 0 &&
            (pDirection ? s->right : s->left)->color == RED) {
            goto del_L5;
        }
        if ((pDirection ? s->left : s->right) != 0 &&
            (pDirection ? s->left : s->right)->color == RED) {
            goto del_L4;
        }

    del_L3:
        s->color = RED;
        p->color = BLACK;
        return found_node;

    del_L4:
        c = (pDirection ? s->left : s->right);
        c->color = BLACK;
        (pDirection ? s->left : s->right) = (pDirection ? c->right : c->left);
        (pDirection ? c->right : c->left) = s;
        (pDirection ? p->right : p->left) = c;
        s->color = RED;
        s = c;

    del_L5:
        s->color = p->color;
        p->color = BLACK;
        (pDirection ? p->right : p->left) = (pDirection ? s->left : s->right);
        (pDirection ? s->left : s->right) = p;
        ((pDirection ? s->right : s->left))->color = BLACK;
        if (--pParent < &parents[0]) {
            root = s;
        } else {
            n = *pParent;
            bool pDirection = p == n->left;
            (pDirection ? n->left : n->right) = s;
        }

        return found_node;
    }

    return found_node;
}

template <class T>
void RBTree<T>::print_tree() {
    unsigned long max_height = 2 * fast_log2(element_count + 2) - 2;
    max_height++;
    unsigned int dist = element_count * 3;

    log("Printing tree:\n");

    for (unsigned int i = 0; i < max_height; i++) {
        print_level(i, dist, root);
        dist /= 2;
        OutputStream::instance << "\n";
    }
    OutputStream::instance << "\n";
}

template <class T>
void RBTree<T>::print_level(unsigned int level, int dist,
                            struct RBTree<T>::node *root) {
    if (level == 0) {
        for (int i = 0; i < dist - 1; i++) {
            OutputStream::instance << " ";
        }
        if (root != 0) {
            OutputStream::instance << (root->color == RED ? "R" : "B");
        } else {
            OutputStream::instance << "O";
        }
        for (int i = 0; i < dist + 1; i++) {
            OutputStream::instance << " ";
        }
    } else {
        print_level(level - 1, dist, root == 0 ? 0 : root->left);
        print_level(level - 1, dist, root == 0 ? 0 : root->right);
    }
}

template <class T>
unsigned long RBTree<T>::get_element_count() {
    return element_count;
}