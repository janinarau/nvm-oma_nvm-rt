#ifndef SYSTEM_DATA_RBTREE
#define SYSTEM_DATA_RBTREE

template <class T>
class Node_Comparator {
   public:
    static int compare(T node1, T node2);
    static bool equals(T node1, T node2);
};

template <class T>
class RBTree {
   private:
    enum node_color { RED, BLACK };

   public:
    RBTree();

    struct node {
        T value;
        node_color color;
        struct node *left;
        struct node *right;
    };

    void insert(struct node *element);
    void remove(struct node *element);
    T pop_minimum();
    unsigned long get_element_count();

    void print_tree();

   private:
    struct node *root = 0;

    unsigned long element_count = 0;

    unsigned long fast_log2(unsigned long n);

    void print_level(unsigned int level, int dist, struct node* root);
};

#endif