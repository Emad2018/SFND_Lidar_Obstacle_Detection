/* \author Aaron Brown */
// Quiz on implementing kd tree

#include "../../render/render.h"

// Structure to represent node of kd tree
struct Node {
  std::vector<float> point;
  int id;
  Node *left;
  Node *right;

  Node(std::vector<float> arr, int setId)
      : point(arr), id(setId), left(NULL), right(NULL) {}
};

struct KdTree {
  Node *root;

  KdTree() : root(NULL) {}
  void add(Node **src, uint depth, std::vector<float> point, int id) {

    if (*src == NULL) {
      *src = new Node(point, id);
    } else {
      uint rimender = depth % 2;

      if (point[rimender] < ((*src)->point[rimender])) {
        add(&((*src)->left), depth + 1, point, id);

      } else {
        add(&((*src)->right), depth + 1, point, id);
      }
    }
  }
  void insert(std::vector<float> point, int id) {
    // TODO: Fill in this function to insert a new point into the tree
    // the function should create a new node and place correctly with in the
    // root
    add(&root, 0, point, id);
  }

  void find(Node **src, uint depth, float *high, float *low,
            std::vector<int> *ids) {

    uint rimender = depth % 2;

    if (*src != NULL) {

      /* std::cout << "(*src)->id " << (*src)->id << "\n";
      std::cout << "point=(" << (*src)->point[0] << "," << (*src)->point[1]
                << ")\n";
      std::cout << "depth= " << depth << "\n"; */

      if (low[rimender] > ((*src)->point[rimender])) {
        // std::cout << "a\n";
        find(&((*src)->right), depth + 1, high, low, ids);

      } else if (high[rimender] < ((*src)->point[rimender])) {
        // std::cout << "b\n";
        find(&((*src)->left), depth + 1, high, low, ids);
      } else {
        // std::cout << "c\n";
        if ((low[!rimender] < ((*src)->point[!rimender])) &&
            (high[!rimender] > ((*src)->point[!rimender]))) {
          ids->push_back((*src)->id);
        }

        find(&((*src)->left), depth + 1, high, low, ids);
        find(&((*src)->right), depth + 1, high, low, ids);
      }
    }
  }
  // return a list of point ids in the tree that are within distance of target
  std::vector<int> search(std::vector<float> target, float distanceTol) {
    std::vector<int> ids;

    uint depth = 0;
    float high[2] = {target[0] + distanceTol, target[1] + distanceTol};
    float low[2] = {target[0] - distanceTol, target[1] - distanceTol};
    // std::cout << "high=(" << high[0] << "," << high[1] << ")\n";
    // std::cout << "low=(" << low[0] << "," << low[1] << ")\n";
    find(&root, 0, high, low, &ids);
    return ids;
  }

  /* extra functions to print the Tree elements and the relativity  */
  void loop(Node **src) {
    if (*src != NULL) {
      std::cout << "(*src)->id " << (*src)->id << "\n";
      std::cout << "point=(" << (*src)->point[0] << "," << (*src)->point[1]
                << ")\n";
      if ((*src)->left != NULL) {
        std::cout << "left node of" << (*src)->id << ":\n";
        loop(&(*src)->left);
      }
      if ((*src)->right != NULL) {
        std::cout << "right node of" << (*src)->id << ":\n";
        loop(&(*src)->right);
      }
    }
  }
  void print_nodes_information() { loop(&root); }
};
