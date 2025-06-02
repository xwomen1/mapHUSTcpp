#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>
#include <sstream>
#include <fstream>
#include"httplib.h"
#include "json.hpp" 


// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// HTTP and JSON libraries

using json = nlohmann::json;

struct Point {
    std::string Name;
    double Lat;
    double Lng;
    int Capacity; // sức chứa tối đa
    int Occupied; // số xe hiện tại (sẽ được cập nhật qua API)
    
    Point() : Name(""), Lat(0.0), Lng(0.0), Capacity(0), Occupied(0) {}
    Point(const std::string& name, double lat, double lng, int capacity, int occupied)
        : Name(name), Lat(lat), Lng(lng), Capacity(capacity), Occupied(occupied) {}
};

struct Coordinate {
    double lat;
    double lng;
    
    Coordinate() : lat(0.0), lng(0.0) {}
    Coordinate(double lat, double lng) : lat(lat), lng(lng) {}
};

// Convert Coordinate to JSON
void to_json(json& j, const Coordinate& c) {
    j = json{{"lat", c.lat}, {"lng", c.lng}};
}

// Convert JSON to Coordinate
void from_json(const json& j, Coordinate& c) {
    j.at("lat").get_to(c.lat);
    j.at("lng").get_to(c.lng);
}

std::unordered_map<std::string, Point> points = {
    {"Nha xe D9", Point("Nha xe D9", 21.004061, 105.844573, 300, 300)},
    {"Nha xe D3-5", Point("Nha xe D3-5", 21.004972, 105.845431, 500, 0)},
    {"Nha xe C7", Point("Nha xe C7", 21.005054, 105.844911, 300, 0)},
    {"Nha xe C5", Point("Nha xe C5", 21.005863, 105.844629, 200, 0)},
    {"Nha xe D4-6", Point("Nha xe D4-6", 21.004592, 105.842322, 300, 0)},
    {"Nha xe B1", Point("Nha xe B1", 21.005002, 105.846058, 100, 0)},
    {"Nha xe TC", Point("Nha xe TC", 21.002553, 105.847055, 500, 0)},
    {"Nha xe B13", Point("Nha xe B13", 21.006460, 105.847312, 100, 0)},
    {"Nha xe B6", Point("Nha xe B6", 21.006319, 105.8465455, 100, 0)},

    // Các ngã rẽ
    {"Intersection D4-D6", Point("Intersection D4-D6", 21.005079, 105.842333, 0, 0)},

    {"Intersection TDN", Point("Intersection TDN", 21.005032, 105.845634, 0, 0)},

    {"Intersection D9-C5", Point("Intersection D9-C5", 21.005027, 105.844605, 0, 0)},
    {"Intersection B6", Point("Intersection B6", 21.005022, 105.8465080, 0, 0)},

    {"Intersection B13TC", Point("Intersection B13TC", 21.004927, 105.846958, 0, 0)},
    {"Intersection TQB-TC", Point("Intersection TQB-TC", 21.003244, 105.847993, 0, 0)},
    {"Intersection TQB1", Point("Intersection TQB1", 21.004501, 105.847210, 0, 0)},
    {"Intersection TQB2", Point("Intersection TQB2", 21.004081, 105.847231, 0, 0)},
    {"Intersection quaydauTC", Point("Intersection quaydauTC", 21.001862, 105.846331, 0, 0)}
};

struct Edge {
    std::string To;
    int Weight;
    
    Edge(const std::string& to, int weight) : To(to), Weight(weight) {}
};

using Graph = std::unordered_map<std::string, std::vector<Edge>>;

Graph graph = {
    // Trục chính
    {"Intersection D4-D6", {Edge("Nha xe D4-6", 2), Edge("Intersection D9-C5", 10)}},
    {"Intersection TDN", {Edge("Nha xe C7", 3), Edge("Nha xe B1", 2)}},
    {"Intersection D9-C5", {Edge("Intersection D4-D6", 10), Edge("Nha xe D9", 4), Edge("Nha xe C5", 4), Edge("Nha xe C7", 1), Edge("Nha xe D3-5", 4)}},
    {"Intersection B6", {Edge("Nha xe B1", 1), Edge("Nha xe B6", 5), Edge("Intersection B13TC", 1)}},
    {"Intersection B13TC", {Edge("Intersection B6", 1), Edge("Intersection TQB1", 1), Edge("Nha xe B13", 6)}},
    {"Intersection TQB-TC", {Edge("Intersection TQB2", 3), Edge("Nha xe TC", 4)}},
    {"Intersection TQB1", {Edge("Intersection B13TC", 6), Edge("Intersection TQB2", 2)}},
    {"Intersection TQB2", {Edge("Intersection TQB1", 2), Edge("Intersection TQB-TC", 2)}},
    {"Intersection quaydauTC", {Edge("Intersection TQB-TC", 1)}},

    // Các nhà xe
    {"Nha xe C7", {Edge("Intersection D9-C5", 1)}},
    {"Nha xe C5", {Edge("Intersection D9-C5", 3)}},
    {"Nha xe D3-5", {Edge("Intersection TDN", 1)}},

    {"Nha xe D9", {Edge("Intersection D9-C5", 3)}},
    {"Nha xe D4-6", {Edge("Intersection D4-D6", 1)}},
    {"Nha xe B1", {Edge("Intersection TDN", 1), Edge("Intersection B6", 1)}},
    {"Nha xe B6", {Edge("Intersection B6", 5)}},
    {"Nha xe TC", {Edge("Intersection quaydauTC", 2)}},
    {"Nha xe B13", {Edge("Intersection B13TC", 6)}}
};

// Dijkstra
struct Item {
    std::string Node;
    int Distance;
    int Index;
    
    Item(const std::string& node, int distance) : Node(node), Distance(distance), Index(0) {}
};

struct ItemComparator {
    bool operator()(const Item& a, const Item& b) const {
        return a.Distance > b.Distance; // Min-heap (reverse comparison)
    }
};

std::pair<std::vector<std::string>, int> Dijkstra(const Graph& graph, const std::string& start, const std::string& end) {
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> prev;
    std::priority_queue<Item, std::vector<Item>, ItemComparator> pq;

    // Initialize distances
    for (const auto& node : graph) {
        dist[node.first] = std::numeric_limits<int>::max();
    }
    dist[start] = 0;
    pq.push(Item(start, 0));

    while (!pq.empty()) {
        Item u = pq.top();
        pq.pop();
        
        if (u.Node == end) {
            break;
        }
        
        auto it = graph.find(u.Node);
        if (it != graph.end()) {
            for (const Edge& edge : it->second) {
                int alt = dist[u.Node] + edge.Weight;
                if (alt < dist[edge.To]) {
                    dist[edge.To] = alt;
                    prev[edge.To] = u.Node;
                    pq.push(Item(edge.To, alt));
                }
            }
        }
    }

    std::vector<std::string> path;
    std::string u = end;
    while (!u.empty()) {
        path.insert(path.begin(), u);
        if (u == start) {
            break;
        }
        auto it = prev.find(u);
        if (it != prev.end()) {
            u = it->second;
        } else {
            break;
        }
    }
    
    return std::make_pair(path, dist[end]);
}

void updateOccupiedHandler(const httplib::Request& req, httplib::Response& res) {
    try {
        json j = json::parse(req.body);
        std::string id = j["id"];
        int occupied = j["occupied"];

        auto it = points.find(id);
        if (it != points.end()) {
            it->second.Occupied = occupied;
            res.status = 200;
            res.set_content("Updated", "text/plain");
        } else {
            res.status = 404;
            res.set_content("Point not found", "text/plain");
        }
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content("invalid request", "text/plain");
    }
}

struct Location {
    double Lat;
    double Lng;
    
    Location() : Lat(0.0), Lng(0.0) {}
    Location(double lat, double lng) : Lat(lat), Lng(lng) {}
};

// Convert JSON to Location
void from_json(const json& j, Location& l) {
    j.at("lat").get_to(l.Lat);
    j.at("lng").get_to(l.Lng);
}

double Haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371e3; // bán kính Trái đất (m)
    double φ1 = lat1 * M_PI / 180;
    double φ2 = lat2 * M_PI / 180;
    double Δφ = (lat2 - lat1) * M_PI / 180;
    double Δλ = (lon2 - lon1) * M_PI / 180;

    double a = std::sin(Δφ/2) * std::sin(Δφ/2) +
               std::cos(φ1) * std::cos(φ2) *
               std::sin(Δλ/2) * std::sin(Δλ/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));

    return R * c; // khoảng cách (mét)
}

std::string FindClosestNodeFromLocation(const Location& loc) {
    double minDist = std::numeric_limits<double>::max();
    std::string closest = "";
    
    for (const auto& p : points) {
        // Chỉ chọn điểm có trong đồ thị (có thể lọc qua graph[id])
        if (graph.find(p.first) != graph.end()) {
            double dist = Haversine(loc.Lat, loc.Lng, p.second.Lat, p.second.Lng);
            if (dist < minDist) {
                minDist = dist;
                closest = p.first;
            }
        }
    }
    return closest;
}

std::pair<std::string, double> FindNearestPoint(const Location& current) {
    double minDist = std::numeric_limits<double>::max();
    std::string nearest = "";

    for (const auto& p : points) {
        double dist = Haversine(current.Lat, current.Lng, p.second.Lat, p.second.Lng);
        if (dist < minDist) {
            minDist = dist;
            nearest = p.first;
        }
    }

    return std::make_pair(nearest, minDist);
}

// Tìm nhà xe gần nhất (còn chỗ) có tổng số đường đi bé nhất với point truyền vào (theo graph)
std::string FindNearestAvailableParking(const std::string& fromID) {
    int minDist = std::numeric_limits<int>::max();
    std::string nearest = "";
    
    for (const auto& p : points) {
        if (p.second.Capacity == 0 || p.second.Occupied >= p.second.Capacity) {
            continue;
        }
        // Tìm đường đi ngắn nhất từ fromID đến id
        auto result = Dijkstra(graph, fromID, p.first);
        int dist = result.second;
        if (dist < minDist) {
            minDist = dist;
            nearest = p.first;
        }
    }
    return nearest;
}

int main() {
    std::cout << "Starting server..." << std::endl;
    httplib::Server server;

    server.Get("/path", [](const httplib::Request& req, httplib::Response& res) {
        std::string from = req.get_param_value("from");
        std::string to = req.get_param_value("to");
        
        auto result = Dijkstra(graph, from, to);
        std::vector<std::string> path = result.first;
        
        std::vector<Coordinate> coordinates;
        for (const std::string& id : path) {
            auto it = points.find(id);
            if (it != points.end()) {
                const Point& p = it->second;
                coordinates.push_back(Coordinate(p.Lat, p.Lng));
            }
        }
        
        json j = coordinates;
        res.set_content(j.dump(), "application/json");
    });

    server.Post("/update-occupied", [](const httplib::Request& req, httplib::Response& res) {
        updateOccupiedHandler(req, res);
    });

    server.Post("/nearest", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json j = json::parse(req.body);
            Location loc;
            from_json(j, loc);
            
            auto nearestResult = FindNearestPoint(loc);
            std::string nearestPointID = nearestResult.first;
            
            if (nearestPointID.empty()) {
                json errorResponse = {{"error", "no available parking"}};
                res.status = 404;
                res.set_content(errorResponse.dump(), "application/json");
                return;
            }

            std::string nearestParkingID = FindNearestAvailableParking(nearestPointID);
            if (nearestParkingID.empty()) {
                json errorResponse = {{"error", "no available parking"}};
                res.status = 404;
                res.set_content(errorResponse.dump(), "application/json");
                return;
            }

            auto pathResult = Dijkstra(graph, nearestPointID, nearestParkingID);
            std::vector<std::string> path = pathResult.first;

            std::vector<Coordinate> coordinates;
            for (const std::string& id : path) {
                auto it = points.find(id);
                if (it != points.end()) {
                    const Point& p = it->second;
                    coordinates.push_back(Coordinate(p.Lat, p.Lng));
                }
            }
            
            json response = {
                {"parkingID", nearestParkingID},
                {"path", coordinates},
                {"status", std::to_string(points[nearestParkingID].Occupied) + "/" + std::to_string(points[nearestParkingID].Capacity) },
               
            };
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            json errorResponse = {{"error", "invalid location"}};
            res.status = 400;
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    server.set_mount_point("/", "./"); // serve frontend
    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::ifstream file("./index.html");
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });

    std::cout << "Server starting on port 8080..." << std::endl;
    server.listen("0.0.0.0", 8080);
    std::cout <<"End main" << std::endl;;

    return 0;
}
