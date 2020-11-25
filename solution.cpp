#include <bits/stdc++.h>

using namespace std;

struct Point {
    int X, Y;
};

struct Angle {
    Point Vertex; //the middle one
    Point Rav1;
    Point Rav2;

    size_t PolygonNumber;

    Angle(Point v, Point r1, Point r2, size_t num): Vertex(v), Rav1(r1), Rav2(r2), PolygonNumber(num) {}
};

typedef vector<Point> Polygon;

struct StraightLine {
    int A, B, C;
    size_t PolyNum;
    bool UpDirection; // shows if inner part of polygon lays above the line

    StraightLine(int a, int b, int c): A(a), B(b), C(c) {}

    StraightLine(Point a, Point b) {
        A = a.Y - b.Y;
        B = b.X - a.X;
        C = a.X * b.Y - a.Y * b.X;
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

//Node is a struct for vertices in nested tree of polygons
struct Node {
    size_t PolygonID;
    vector<Node *> children;
    Node *parent = nullptr;

    bool OnEvenLevel = false; // shows if node lays on even level, when we numerate levels of the tree from 1
};

Node *getRoot(Node *node) {
    while (node->parent != nullptr) {
        node = node->parent;
    }
    return node;
}

// walk pass through the tree of polygons and saves "components" from the tree
// node - the beginning of walk (root)
// comps - result with components
// passed - vector to mark passed nodes
void walk(Node *node, vector<vector<size_t>>& comps, vector<bool>& passed) {
    queue<Node *> q;
    q.push(getRoot(node));
    while (!q.empty()) {
        auto curentNode = q.front();
        q.pop();
        passed[curentNode->PolygonID] = true;
        if (!curentNode->OnEvenLevel) {
            vector<size_t> component;
            component.push_back(curentNode->PolygonID);
            for (auto child : curentNode->children) {
                component.push_back(child->PolygonID);
            }
            comps.push_back(component);
        }
        for (auto child : curentNode->children) {
            child->OnEvenLevel = !curentNode->OnEvenLevel;
            q.push(child);
        }
    }
}

// parseTree get all components from all trees
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

// getAngles returnes all angles of all polygons
void getAngles(const vector<Polygon>& polygons, vector<Angle>& angles) {
    int allPointsNumber = 0;
    for (Polygon p : polygons) {
        allPointsNumber += p.size();
    }
    angles.reserve(allPointsNumber);

    for (size_t i = 0; i < polygons.size(); ++i) {
        Polygon polygon = polygons[i];
        for (size_t j = 0; j < polygon.size(); ++j) { // consider polygons to be correct, so there is 3 or more points in it
            if (j == 0) {
                angles.push_back(Angle(polygon[j], polygon.back(), polygon[j + 1], i));
            } else if (j < polygon.size() - 1) {
                angles.push_back(Angle(polygon[j], polygon[j - 1], polygon[j + 1], i));
            } else {
                angles.push_back(Angle(polygon[j], polygon[j - 1], polygon.front(), i));
            }
        }
    }
}

// updateBothLines is used to add lines of polygon's angle formed by the current point and it's neighbours
// and fix the direction of inner part of polygon for both lines
// polygonAngle - current angle of polygon
// lines - pass set of StraightLines with special comparator
// x - current Ox, here we pass x which is used in comparator
// inside - shows if two lines of angle have inner part of polygon between these lines, so lines "look" inside
template <typename SetType>
void updateBothLines(Angle polygonAngle, SetType& lines, int& x, bool inside) {
    Angle angle =  polygonAngle;
    StraightLine up = StraightLine(angle.Vertex, angle.Rav1);
    StraightLine down = StraightLine(angle.Vertex, angle.Rav2);
    if (down.getY(angle.Rav1.X) > double(angle.Rav1.Y)) {
        swap(up, down);
    }
    if (inside) {
        up.UpDirection = false;
        down.UpDirection = true;
    } else {
        up.UpDirection = true;
        down.UpDirection = false;
    }
    up.PolyNum = polygonAngle.PolygonNumber;
    down.PolyNum = polygonAngle.PolygonNumber;
    x += 1; // for lines to not be deleted
    lines.insert(up);
    lines.insert(down);
}

// updateFirstLine is used to add line of polygon's angle formed by the current point and it's neighbour
// and fix the direction of inner part of polygon for line
// polygonAngle - current angle of polygon
// lines - pass set of StraightLines with special comparator
// upside - shows if line of angle have inner part of polygon above, so line "looks" upside
template <typename SetType>
void updateFirstLine(Angle polygonAngle, SetType& lines, bool upside) {
    Angle angle =  polygonAngle;
    StraightLine up = StraightLine(angle.Vertex, angle.Rav1);
    up.UpDirection = upside;
    up.PolyNum = polygonAngle.PolygonNumber;
    lines.insert(up);
}

// updateSecondLine is used to add line of polygon's angle formed by the current point and it's neighbour
// and fix the direction of inner part of polygon for line
// polygonAngle - current angle of polygon
// lines - pass set of StraightLines with special comparator
// upside - shows if line of angle have inner part of polygon above, so line "looks" upside
template <typename SetType>
void updateSecondLine(Angle polygonAngle, SetType& lines, bool upside) {
    Angle angle =  polygonAngle;
    StraightLine up = StraightLine(angle.Vertex, angle.Rav2);
    up.UpDirection = upside;
    up.PolyNum = polygonAngle.PolygonNumber;
    lines.insert(up);
}

// updateTree adds new verticle for polygon if it doesn't exist
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

// updateLines checks which lines of the angle start at the point and need to be added
// polygonAngle - current angle of polygon
// lines - pass set of StraightLines with special comparator
// x - current Ox, here we pass x which is used in comparator
// inside - shows if two lines of angle have inner part of polygon between these lines, needed for both lines case
// upside - shows if line of angle have inner part of polygon above, needed for one line case
template <typename SetType>
void updateLines(Angle polygonAngle, SetType& lines, int& x, bool inside, bool upside) {
    Angle angle =  polygonAngle;
    if (angle.Rav1.X > angle.Vertex.X && angle.Rav2.X > angle.Vertex.X) {

        updateBothLines(polygonAngle, lines, x, inside);

    } else if (angle.Rav1.X > angle.Vertex.X && angle.Rav2.X <= angle.Vertex.X) {

        updateFirstLine(polygonAngle, lines, upside);

    } else if (angle.Rav2.X > angle.Vertex.X && angle.Rav1.X <= angle.Vertex.X) {

        updateSecondLine(polygonAngle, lines, upside);

    }
}

// getComponents goes through sorted angles of polygons and at each point draws imaginary line
// parallel to Oy; at that line we look after two closest lines to our point from above and below
// and consider different cases to add new lines and remove lines that are ended at new point
vector<vector<size_t>> getComponents(const vector<Polygon>& polygons) {
    vector<Angle> angles; // stores angle (3 connected points) and polygon number
    getAngles(polygons, angles);

    int x = 0;
    auto comparator = [&x](const StraightLine& a, const StraightLine& b) {
        return a.getY(x) < b.getY(x);
    };
    set<StraightLine, decltype(comparator)> lines(comparator);
    sort(angles.begin(), angles.end(), [](const Angle& a, const Angle& b) {
        return a.Vertex.X < b.Vertex.X || a.Vertex.X == b.Vertex.X && a.Vertex.Y < b.Vertex.Y;
    });
    vector<Node *> tree(polygons.size(), nullptr);

    for (auto polygonAngle : angles) {
        //cout << "DEBUG " << polygonAngle.Vertex.X << " " << polygonAngle.Vertex.Y << endl;
        Angle angle = polygonAngle;
        x = angle.Vertex.X;
        if (lines.empty()) {
            
            updateLines(polygonAngle, lines, x, true, true);
            updateTree(tree, polygonAngle.PolygonNumber, nullptr);
            
            continue;
        }

        // find bounds
        lines.erase(StraightLine(0, 1, -angle.Vertex.Y));
        auto greater = lines.upper_bound(StraightLine(0, 1, -angle.Vertex.Y));

        if (greater != lines.end() && greater != lines.begin()) {
            auto less = prev(greater);
            if ((*less).PolyNum == (*greater).PolyNum) { // lines from the same polygon
                if ((*less).UpDirection && !(*greater).UpDirection) { // both lines look inside
                    if ((*less).PolyNum == polygonAngle.PolygonNumber) { // the point of polygon inside of polygon

                        updateLines(polygonAngle, lines, x, false, false);

                    } else { // the point is inside polygon but from other polygon

                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.PolygonNumber, tree[(*less).PolyNum]);
                    }
                } else if ((*less).UpDirection && (*greater).UpDirection) {

                    updateLines(polygonAngle, lines, x, false, false); // in this case first condition of updateLines is imposible

                } else if (!(*less).UpDirection && !(*greater).UpDirection) {

                    updateLines(polygonAngle, lines, x, true, true); // in this case first condition of updateLines is imposible

                } else {
                    if ((*less).PolyNum == polygonAngle.PolygonNumber) { // the point of polygon outside of polygon

                        updateLines(polygonAngle, lines, x, true, true);

                    } else { // the point is outside polygon but from other polygon
                        
                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.PolygonNumber, tree[(*less).PolyNum]->parent);
                    }
                }
            } else { // lines from different polygons
                if ((*less).UpDirection && !(*greater).UpDirection) { // both lines look inside
                    if ((*less).PolyNum == polygonAngle.PolygonNumber) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, false, false); // in this case first condition of updateLines is imposible

                    } else if ((*greater).PolyNum == polygonAngle.PolygonNumber) {
                        
                        updateLines(polygonAngle, lines, x, true, true); // in this case first condition of updateLines is imposible

                    }
                } else if ((*less).UpDirection && (*greater).UpDirection) {
                    if ((*less).PolyNum == polygonAngle.PolygonNumber) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, false, false);

                    } else if ((*greater).PolyNum == polygonAngle.PolygonNumber) {
                        
                        updateLines(polygonAngle, lines, x, true, true);

                    } else {
                        
                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.PolygonNumber, tree[(*less).PolyNum]);
                    }
                } else if (!(*less).UpDirection && !(*greater).UpDirection) {
                    if ((*greater).PolyNum == polygonAngle.PolygonNumber) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, false, true);

                    } else if ((*less).PolyNum == polygonAngle.PolygonNumber) {
                        
                        updateLines(polygonAngle, lines, x, true, true);

                    } else {
                        
                        updateLines(polygonAngle, lines, x, true, true);
                        updateTree(tree, polygonAngle.PolygonNumber, tree[(*greater).PolyNum]);
                    }
                } else { // lines look outside
                    if ((*greater).PolyNum == polygonAngle.PolygonNumber) { // the point of polygon on the boarder with another polygon
                        
                        updateLines(polygonAngle, lines, x, true, false);

                    } else if ((*less).PolyNum == polygonAngle.PolygonNumber) {
                        
                        updateLines(polygonAngle, lines, x, true, true);

                    } else { // point and lines are from different polygons
                        updateLines(
                                polygonAngle,
                                lines,
                                x,
                                tree[polygonAngle.PolygonNumber] == nullptr,
                                tree[(*less).PolyNum]->parent != tree[polygonAngle.PolygonNumber]
                        );
                        updateTree(tree, polygonAngle.PolygonNumber, tree[(*greater).PolyNum]->parent);
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
            updateTree(tree, polygonAngle.PolygonNumber, nullptr);
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
