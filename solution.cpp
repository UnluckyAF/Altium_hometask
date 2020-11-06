#include <bits/stdc++.h>

using namespace std;

typedef pair<int, int> Point;
typedef vector<Point> Polygon;
typedef pair<Point, pair<Point, Point>> Angle;

struct StraightLine {
    int A, B, C;
    size_t PolyNum;
    bool UpDirection;

    StraightLine(int a, int b, int c): A(a), B(b), C(c) {}

    StraightLine(Point a, Point b) {
        A = a.second - b.second;
        B = b.first - a.first;
        C = a.first * b.second - a.second * b.first;
    }

    double getY(int x) const {
        return (-C - A * x) / double(B);
    }
};

void input(istream& in, vector<Polygon>& polygons) {
    int polygonsNumber;
    in >> polygonsNumber;
    polygons.reserve(polygonsNumber);

    for (int i = 0; i < polygonsNumber; ++i) {
        int pointsNumber;
        in >> pointsNumber;
        Polygon polygon;
        polygon.reserve(pointsNumber); 

        for (int j = 0; j < pointsNumber; ++j) {
            int x, y;
            in >> x >> y;
            polygon.push_back({x, y});
        }
        polygons.push_back(polygon);
    }
}

struct Node {
    size_t PolygonID;
    vector<Node *> children;
    Node *parent = nullptr;
};

Node *getRoot(Node *node) {
    while (node->parent != nullptr) {
        node = node->parent;
    }
    return node;
}

// walk pass through the tree of polygons and saves "components" from the tree
void walk(Node *node, vector<vector<size_t>>& comps, vector<bool>& passed) {
    queue<pair<Node *, bool>> q;
    q.push({getRoot(node), false});
    while (!q.empty()) {
        auto curentNode = q.front();
        q.pop();
        passed[curentNode.first->PolygonID] = true;
        if (!curentNode.second) {
            vector<size_t> component;
            component.push_back(curentNode.first->PolygonID);
            for (auto child : curentNode.first->children) {
                component.push_back(child->PolygonID);
            }
            comps.push_back(component);
        }
        for (auto child : curentNode.first->children) {
            q.push({child, !curentNode.second});
        }
    }
}

vector<vector<size_t>> parseTree(const vector<Node *>& tree) {
    vector<vector<size_t>> components;
    vector<bool> used(tree.size(), false);
    for (size_t i = 0; i < tree.size(); ++i) {
        if (!used[i]) {
            walk(tree[i], components, used);
        }
        delete(tree[i]);
    }
    return components;
}

void getAngles(const vector<Polygon>& polygons, vector<pair<Angle, size_t>>& angles) {
    int allPointsNumber = 0;
    for (Polygon p : polygons) {
        allPointsNumber += p.size();
    }
    angles.reserve(allPointsNumber);

    for (size_t i = 0; i < polygons.size(); ++i) {
        Polygon polygon = polygons[i];
        for (size_t j = 0; j < polygon.size(); ++j) { // consider polygons to be correct, so there is 3 or more points in it
            if (j == 0) {
                angles.push_back({{polygon[j], {polygon.back(), polygon[j + 1]}}, i});
            } else if (j < polygon.size() - 1) {
                angles.push_back({{polygon[j], {polygon[j - 1], polygon[j + 1]}}, i});
            } else {
                angles.push_back({{polygon[j], {polygon[j - 1], polygon.front()}}, i});
            }
        }
    }
}

template <typename SetType>
void updateBothLines(pair<Angle, size_t> polygonAngle, SetType& lines, int& x, bool inside) {
    Angle angle =  polygonAngle.first;
    StraightLine up = StraightLine(angle.first, angle.second.first);
    StraightLine down = StraightLine(angle.first, angle.second.second);
    if (down.getY(angle.second.first.first) > double(angle.second.first.second)) {
        swap(up, down);
    }
    if (inside) {
        up.UpDirection = false;
        down.UpDirection = true;
    } else {
        up.UpDirection = true;
        down.UpDirection = false;
    }
    up.PolyNum = polygonAngle.second;
    down.PolyNum = polygonAngle.second;
    x += 1; // for lines to not be deleted
    lines.insert(up);
    lines.insert(down);
}

template <typename SetType>
void updateFirstLine(pair<Angle, size_t> polygonAngle, SetType& lines, bool upside) {
    Angle angle =  polygonAngle.first;
    StraightLine up = StraightLine(angle.first, angle.second.first);
    up.UpDirection = upside;
    up.PolyNum = polygonAngle.second;
    lines.insert(up);
}

template <typename SetType>
void updateSecondLine(pair<Angle, size_t> polygonAngle, SetType& lines, bool upside) {
    Angle angle =  polygonAngle.first;
    StraightLine up = StraightLine(angle.first, angle.second.second);
    up.UpDirection = upside;
    up.PolyNum = polygonAngle.second;
    lines.insert(up);
}

void updateTree(vector<Node *>& tree, size_t polygonID, Node *parent) {
    if (tree[polygonID] == nullptr) {
        Node *node = new Node;
        node->PolygonID = polygonID;
        tree[polygonID] = node;
        if (parent != nullptr) {
            node->parent = parent;
            parent->children.push_back(node);
        }
    }
}

template <typename SetType>
void updateLines(pair<Angle, size_t> polygonAngle, SetType& lines, int& x, bool inside, bool upside) {
    Angle angle =  polygonAngle.first;
    if (angle.second.first.first > angle.first.first && angle.second.second.first > angle.first.first) {
        updateBothLines(polygonAngle, lines, x, inside);
    } else if (angle.second.first.first > angle.first.first && angle.second.second.first <= angle.first.first) {
        updateFirstLine(polygonAngle, lines, upside);
    } else if (angle.second.second.first > angle.first.first && angle.second.first.first <= angle.first.first) {
        updateSecondLine(polygonAngle, lines, upside);
    }
}

vector<vector<size_t>> getComponents(const vector<Polygon>& polygons) {
    vector<pair<Angle, size_t>> angles; // stores angle (3 connected points) and polygon number
    getAngles(polygons, angles);

    int x = 0;
    auto comparator = [&x](const StraightLine& a, const StraightLine& b) {
        return a.getY(x) < b.getY(x);
    };
    set<StraightLine, decltype(comparator)> lines(comparator);
    sort(angles.begin(), angles.end(), [](const pair<Angle, size_t>& a, const pair<Angle, size_t>& b) {
        return a.first.first < b.first.first;
    });
    vector<Node *> tree(polygons.size(), nullptr);

    for (auto polygonAngle : angles) {
        Angle angle = polygonAngle.first;
        x = angle.first.first;
        if (lines.empty()) {
            
            updateLines(polygonAngle, lines, x, true, true);
            updateTree(tree, polygonAngle.second, nullptr);
            
            continue;
        }

        // find bounds
        lines.erase(StraightLine(0, 1, -angle.first.second));
        auto greater = lines.upper_bound(StraightLine(0, 1, -angle.first.second));

        if (greater != lines.end() && greater != lines.begin()) {
            auto less = prev(greater);
            if ((*less).PolyNum == (*greater).PolyNum) { // lines from the same polygon
                if ((*less).UpDirection && !(*greater).UpDirection) { // both lines look inside
                    if ((*less).PolyNum == polygonAngle.second) { // the point of polygon inside of polygon

                        updateLines(polygonAngle, lines, x, false, false);

                    } else { // the point is inside polygon but from other polygon

                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.second, tree[(*less).PolyNum]);
                    }
                } else if ((*less).UpDirection && (*greater).UpDirection) {

                    updateLines(polygonAngle, lines, x, false, false); // in this case first condition of updateLines is imposible

                } else if (!(*less).UpDirection && !(*greater).UpDirection) {

                    updateLines(polygonAngle, lines, x, true, true); // in this case first condition of updateLines is imposible

                } else {
                    if ((*less).PolyNum == polygonAngle.second) { // the point of polygon outside of polygon

                        updateLines(polygonAngle, lines, x, true, true);

                    } else { // the point is outside polygon but from other polygon
                        
                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.second, tree[(*less).PolyNum]->parent);
                    }
                }
            } else { // lines from different polygons
                if ((*less).UpDirection && !(*greater).UpDirection) { // both lines look inside
                    if ((*less).PolyNum == polygonAngle.second) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, false, false); // in this case first condition of updateLines is imposible

                    } else if ((*greater).PolyNum == polygonAngle.second) {
                        
                        updateLines(polygonAngle, lines, x, true, true); // in this case first condition of updateLines is imposible

                    }
                } else if ((*less).UpDirection && (*greater).UpDirection) {
                    if ((*less).PolyNum == polygonAngle.second) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, false, false);

                    } else if ((*greater).PolyNum == polygonAngle.second) {
                        
                        updateLines(polygonAngle, lines, x, true, true);

                    } else {
                        
                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.second, tree[(*less).PolyNum]);
                    }
                } else if (!(*less).UpDirection && !(*greater).UpDirection) {
                    if ((*greater).PolyNum == polygonAngle.second) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, false, true);

                    } else if ((*less).PolyNum == polygonAngle.second) {
                        
                        updateLines(polygonAngle, lines, x, true, true);

                    } else {
                        
                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.second, tree[(*greater).PolyNum]);
                    }
                } else { // lines look outside
                    if ((*greater).PolyNum == polygonAngle.second) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, true, false);

                    } else if ((*less).PolyNum == polygonAngle.second) {
                        
                        updateLines(polygonAngle, lines, x, true, true);

                    } else { // point and lines are from different polygons
                        updateLines(
                                polygonAngle,
                                lines,
                                x,
                                tree[polygonAngle.second] == nullptr,
                                tree[(*less).PolyNum]->parent != tree[polygonAngle.second]
                        );
                        updateTree(tree, polygonAngle.second, tree[(*greater).PolyNum]->parent);
                    }
                }
            }
        } else {
            if (!lines.empty()) {
                bool inside;
                if (greater != lines.end()) {
                    if ((*greater).UpDirection) {
                        inside = true;
                    } else {
                        inside = false;
                    }
                } else {
                    auto less = prev(greater);
                    if ((*less).UpDirection) {
                        inside = false;
                    } else {
                        inside = true;
                    }
                }
                updateLines(polygonAngle, lines, x, inside, greater != lines.end());
            }
            updateTree(tree, polygonAngle.second, nullptr);
        }
    }

    return parseTree(tree);
}

int main(int argc, char **argv) {
    vector<Polygon> polygons;
    if (argc < 2) {
        cout << R"(Input format:
n - polygons number
then for every polygon:
    k - points number
    for every point:
        x y - coordinates on Ox and Oy
Example:
2
3
1 2
2 3
2 2
4
0 0
0 1
1 1
1 0)";

        input(cin, polygons);
    } else {
        ifstream file(argv[1]);
        if (file) {
            input(file, polygons);
        } else {
            cerr << "ERROR: Can't open file " << argv[1] << " for reading.";
        }
    }
    vector<vector<size_t>> res = getComponents(polygons);
    for (auto v : res) {
        for (auto p : v) {
            cout << p << " ";
        }
        cout << "\n";
    }
}
