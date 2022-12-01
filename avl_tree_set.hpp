#ifndef JOB_SHOP_AVL_TREE_SET
#define JOB_SHOP_AVL_TREE_SET

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

#include <algorithm>
#include <concepts>
#include <string>
#include <utility>

namespace zr {

    template<typename Tp>
    concept less_comparable = requires(Tp a, Tp b) {
        { a < b } -> std::convertible_to<bool>;
    };

    template<typename Tp>
    concept greater_comparable = requires(Tp a, Tp b) {
        { a > b } -> std::convertible_to<bool>;
    };

    template<typename Tp, typename Hash>
    concept hashable = requires(Tp v) {
        { v.hash() } -> std::convertible_to<Hash>;
    };

    template<typename Tp> requires less_comparable<Tp> and greater_comparable<Tp> and std::equality_comparable<Tp>
    class avl_tree_set {

        class tree_node {
        public:
            Tp value;
            tree_node* left = nullptr;
            tree_node* right = nullptr;
            int32_t height = 1;

            explicit tree_node(Tp value) : value(value) {}
        };

        tree_node* root = nullptr;

        int32_t height(tree_node* node) {
            if (node == nullptr) return 0;
            return node->height;
        }

        tree_node* new_node(const Tp& value) {
            auto* node = new tree_node(value);
            return (node);
        }

        tree_node* right_rotate(tree_node* y) {
            tree_node* yl = y->left;
            tree_node* ylr = yl->right;
            yl->right = y;
            y->left = ylr;
            y->height = std::max(height(y->left), height(y->right)) + 1;
            yl->height = std::max(height(yl->left), height(yl->right)) + 1;
            return yl;
        }

        tree_node* left_rotate(tree_node* x) {
            tree_node* xr = x->right;
            tree_node* xrl = xr->left;
            xr->left = x;
            x->right = xrl;
            x->height = std::max(height(x->left), height(x->right)) + 1;
            xr->height = std::max(height(xr->left), height(xr->right)) + 1;
            return xr;
        }

        int32_t get_balance_factor(tree_node* node) {
            if (node == nullptr) return 0;
            return height(node->left) - height(node->right);
        }

        tree_node* insert_node(tree_node* node, const Tp& key) {

            if (node == nullptr) return (new_node(key));
            if (key < node->value) node->left = insert_node(node->left, key);
            else if (key > node->value) node->right = insert_node(node->right, key);
            else return node;

            node->height = 1 + std::max(height(node->left),
                                        height(node->right));
            const int32_t balance_factor = get_balance_factor(node);
            if (balance_factor > 1) {
                if (key < node->left->value) return right_rotate(node);
                else if (key > node->left->value) {
                    node->left = left_rotate(node->left);
                    return right_rotate(node);
                }
            }
            if (balance_factor < -1) {
                if (key > node->right->value) return left_rotate(node);
                else if (key < node->right->value) {
                    node->right = right_rotate(node->right);
                    return left_rotate(node);
                }
            }
            return node;
        }

        void clear(tree_node* node) {
            if (node == nullptr) return;
            clear(node->left);
            clear(node->right);
            delete node;
        }

    public:

        typedef Tp value_type;

        ~avl_tree_set() {
            clear(root);
        }

        void insert(value_type&& key) {
            root = insert_node(std::move(root), key);
        }

        template<typename V>
        requires hashable<V, Tp>
        void insert(V&& key) {
            root = insert_node(std::move(root), key.hash());
        }

        [[nodiscard]] bool contains(value_type&& key) const {
            tree_node* current = root;
            while (current != nullptr) {
                if (key < current->value) current = current->left;
                else if (key > current->value) current = current->right;
                else return true;
            }
            return false;
        }

        template<typename V>
        requires hashable<V, Tp>
        [[nodiscard]] bool contains(V&& key) const {
            return contains(key.hash());
        }
    };
}

#pragma clang diagnostic pop

#endif //JOB_SHOP_AVL_TREE_SET
