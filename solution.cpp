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
    
    for (auto p : angles) {
        Angle a = p.first;
        x = a.first.first;
        if (lines.empty()) {
            if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                StraightLine up = StraightLine(a.first, a.second.first);
                StraightLine down = StraightLine(a.first, a.second.second);
                if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                    swap(up, down);
                }
                up.UpDirection = false;
                down.UpDirection = true;
                up.PolyNum = p.second;
                down.PolyNum = p.second;
                x += 1; // for lines to not be deleted
                lines.insert(up);
                lines.insert(down);
            } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                StraightLine up = StraightLine(a.first, a.second.first);
                up.UpDirection = true;
                up.PolyNum = p.second;
                lines.insert(up);
            } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                StraightLine up = StraightLine(a.first, a.second.second);
                up.UpDirection = true;
                up.PolyNum = p.second;
                lines.insert(up);
            }
            if (tree[p.second] == nullptr) {
                Node *node = new Node;
                node->PolygonID = p.second;
                tree[p.second] = node;
            }
            continue;
        }

        // find bounds
        lines.erase(StraightLine(0, 1, -a.first.second));
        auto greater = lines.upper_bound(StraightLine(0, 1, -a.first.second));

        if (greater != lines.end() && greater != lines.begin()) {
            auto less = prev(greater);
            if ((*less).PolyNum == (*greater).PolyNum) { // lines from the same polygon
                if ((*less).UpDirection && !(*greater).UpDirection) { // both lines look inside
                    if ((*less).PolyNum == p.second) { // the point of polygon inside of polygon
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = true;
                            down.UpDirection = false;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else { // the point is inside polygon but from other polygon
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }//TODO: add conditions

                        if (tree[p.second] == nullptr) {
                            Node *node = new Node;
                            node->PolygonID = p.second;
                            tree[p.second] = node;
                            tree[(*less).PolyNum]->children.push_back(node);
                            node->parent = tree[(*less).PolyNum];
                        }
                    }
                } else if ((*less).UpDirection && (*greater).UpDirection) {
                    if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                        StraightLine up = StraightLine(a.first, a.second.first);
                        up.UpDirection = false;
                        up.PolyNum = p.second;
                        lines.insert(up);
                    } else {
                        StraightLine up = StraightLine(a.first, a.second.second);
                        up.UpDirection = false;
                        up.PolyNum = p.second;
                        lines.insert(up);
                    }
                } else if (!(*less).UpDirection && !(*greater).UpDirection) {
                    if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                        StraightLine up = StraightLine(a.first, a.second.first);
                        up.UpDirection = true;
                        up.PolyNum = p.second;
                        lines.insert(up);
                    } else {
                        StraightLine up = StraightLine(a.first, a.second.second);
                        up.UpDirection = true;
                        up.PolyNum = p.second;
                        lines.insert(up);
                    }
                } else {
                    if ((*less).PolyNum == p.second) { // the point of polygon outside of polygon
                        // TODO: probably with 2 more conditions
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else { // the point is outside polygon but from other polygon
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) { 
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                        
                        if (tree[p.second] == nullptr) {
                            Node *node = new Node();
                            node->PolygonID = p.second;
                            tree[p.second] = node;
                            if (tree[(*less).PolyNum]->parent != nullptr) {
                                node->parent = tree[(*less).PolyNum]->parent;
                                node->parent->children.push_back(node);
                            }
                        }
                    }
                }
            } else { // lines from different polygons
                if ((*less).UpDirection && !(*greater).UpDirection) { // both lines look inside
                    if ((*less).PolyNum == p.second) { // the point of polygon on the boarder with another polygon
                        if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) { // in case above there are only two possible ways
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else if ((*greater).PolyNum == p.second) {
                        if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) { // in case above there are only two possible ways
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    }
                } else if ((*less).UpDirection && (*greater).UpDirection) {
                    if ((*less).PolyNum == p.second) { // the point of polygon on the boarder with another polygon
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = true;
                            down.UpDirection = false;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else if ((*greater).PolyNum == p.second) {
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else {
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }

                        if (tree[p.second] == nullptr) {
                            Node *node = new Node;
                            node->PolygonID = p.second;
                            tree[p.second] = node;
                            tree[(*less).PolyNum]->children.push_back(node);
                            node->parent = tree[(*less).PolyNum];
                        }
                    }
                } else if (!(*less).UpDirection && !(*greater).UpDirection) {
                    if ((*greater).PolyNum == p.second) { // the point of polygon on the boarder with another polygon
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = true;
                            down.UpDirection = false;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else if ((*less).PolyNum == p.second) {
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else {
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }

                        if (tree[p.second] == nullptr) {
                            Node *node = new Node;
                            node->PolygonID = p.second;
                            tree[p.second] = node;
                            tree[(*greater).PolyNum]->children.push_back(node);
                            node->parent = tree[(*greater).PolyNum];
                        }
                    }
                } else { // lines look outside
                    if ((*greater).PolyNum == p.second) { // the point of polygon on the boarder with another polygon
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = false;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else if ((*less).PolyNum == p.second) {
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            up.UpDirection = false;
                            down.UpDirection = true;
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            up.UpDirection = true;
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }
                    } else { // point and lines are from different polygons
                        if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            StraightLine down = StraightLine(a.first, a.second.second);
                            if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                                swap(up, down);
                            }
                            if (tree[p.second] == nullptr) { // this condition at the same time represents that polygon is not parent of two closest polygons
                                up.UpDirection = false;
                                down.UpDirection = true;
                            } else {
                                up.UpDirection = true;
                                down.UpDirection = false;
                            }
                            up.PolyNum = p.second;
                            down.PolyNum = p.second;
                            x += 1; // for lines to not be deleted
                            lines.insert(up);
                            lines.insert(down);
                        } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.first);
                            if (tree[(*less).PolyNum]->parent == tree[p.second]) {
                                up.UpDirection = false;
                            } else {
                                up.UpDirection = true;
                            }
                            up.PolyNum = p.second;
                            lines.insert(up);
                        } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                            StraightLine up = StraightLine(a.first, a.second.second);
                            if (tree[(*less).PolyNum]->parent == tree[p.second]) {
                                up.UpDirection = false;
                            } else {
                                up.UpDirection = true;
                            }
                            up.PolyNum = p.second;
                            lines.insert(up);
                        }

                        if (tree[p.second] == nullptr) {
                            Node *node = new Node;
                            node->PolygonID = p.second;
                            tree[p.second] = node;
                            if (tree[(*greater).PolyNum]->parent != nullptr) {
                                node->parent = tree[(*greater).PolyNum]->parent;
                                node->parent->children.push_back(node);
                            }
                        }
                    }
                }
            }
        } else {
            if (a.second.first.first > a.first.first && a.second.second.first > a.first.first) {
                StraightLine up = StraightLine(a.first, a.second.first);
                StraightLine down = StraightLine(a.first, a.second.second);
                if (down.getY(a.second.first.first) > double(a.second.first.second)) {
                    swap(up, down);
                }
                if (greater != lines.end()) {
                    if ((*greater).UpDirection) {
                        up.UpDirection = false;
                        down.UpDirection = true;
                    } else {
                        up.UpDirection = true;
                        down.UpDirection = false;
                    }
                } else {
                    auto less = prev(greater);
                    if ((*less).UpDirection) {
                        up.UpDirection = true;
                        down.UpDirection = false;
                    } else {
                        up.UpDirection = false;
                        down.UpDirection = true;
                    }
                }
                up.PolyNum = p.second;
                down.PolyNum = p.second;
                x += 1; // for lines to not be deleted
                lines.insert(up);
                lines.insert(down);
            } else if (a.second.first.first > a.first.first && a.second.second.first <= a.first.first) {
                StraightLine up = StraightLine(a.first, a.second.first);
                if (greater != lines.end()) {
                    up.UpDirection = true;
                } else {
                    up.UpDirection = false;
                }
                up.PolyNum = p.second;
                lines.insert(up);
            } else if (a.second.second.first > a.first.first && a.second.first.first <= a.first.first) {
                StraightLine up = StraightLine(a.first, a.second.second);
                if (greater != lines.end()) {
                    up.UpDirection = true;
                } else {
                    up.UpDirection = false;
                }
                up.PolyNum = p.second;
                lines.insert(up);
            }
            if (tree[p.second] == nullptr) {
                Node *node = new Node;
                node->PolygonID = p.second;
                tree[p.second] = node;
            }
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
