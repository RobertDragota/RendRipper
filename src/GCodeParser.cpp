#include "GCodeParser.h"
#include <fstream>
#include <regex>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <stdexcept>

void GCodeParser::Parse(const std::string &path,
                        std::vector<std::vector<GCodeColoredVertex>> &layers,
                        std::vector<float> &layerZs) const
{
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to open G-code file: " + path);
    }

    std::string line;
    glm::vec3 lastPos(0.0f);
    bool hasLastPos = false;
    float lastExtrusion = 0.0f;
    float currentZ = 0.0f;
    int currentLayerIndex = -1;
    glm::vec3 modelColor(0.8f, 0.8f, 0.8f);
    glm::vec3 infillColor(0.9f, 0.4f, 0.1f);
    glm::vec3 supportColor(0.1f, 0.5f, 0.9f);
    glm::vec3 currentColor = modelColor;

    std::regex moveRegex(R"(^(?:G0|G1)\s+([^;]*))");

    while (std::getline(in, line)) {
        std::string comment;
        size_t semiPos = line.find(';');
        if (semiPos != std::string::npos) {
            comment = line.substr(semiPos + 1);
        }

        std::smatch m;
        if (!std::regex_search(line, m, moveRegex)) {
            if (!comment.empty()) {
                std::string upper = comment;
                std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                auto pos = upper.find("TYPE:");
                if (pos != std::string::npos) {
                    std::string type = upper.substr(pos + 5);
                    if (type.find("SUPPORT") != std::string::npos) currentColor = supportColor;
                    else if (type.find("FILL") != std::string::npos) currentColor = infillColor;
                    else currentColor = modelColor;
                }
            }
            continue;
        }

        if (!comment.empty()) {
            std::string upper = comment;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            auto pos = upper.find("TYPE:");
            if (pos != std::string::npos) {
                std::string type = upper.substr(pos + 5);
                if (type.find("SUPPORT") != std::string::npos) currentColor = supportColor;
                else if (type.find("FILL") != std::string::npos) currentColor = infillColor;
                else currentColor = modelColor;
            }
        }

        std::string tokenStr = m[1].str();
        std::istringstream tokenStream(tokenStr);
        std::string token;
        float X = NAN, Y = NAN, Z = NAN, E = NAN;
        while (tokenStream >> token) {
            if (token.size() < 2) continue;
            char prefix = token[0];
            std::string numberPart = token.substr(1);
            float value = 0.0f; bool ok = false;
            try {
                value = std::stof(numberPart); ok = true;
            } catch (const std::exception&) { ok = false; }
            if (!ok) continue;
            switch (prefix) {
                case 'X': X = value; break;
                case 'Y': Y = value; break;
                case 'Z': Z = value; break;
                case 'E': E = value; break;
                default: break;
            }
        }

        if (!std::isnan(Z) && Z != currentZ) {
            currentZ = Z;
            layerZs.push_back(currentZ);
            layers.emplace_back();
            currentLayerIndex = static_cast<int>(layers.size()) - 1;
        }

        glm::vec3 currentPos = lastPos;
        if (!std::isnan(X)) currentPos.x = X;
        if (!std::isnan(Y)) currentPos.y = Y;
        if (!std::isnan(Z)) currentPos.z = Z;

        if (!std::isnan(E) && E > lastExtrusion && hasLastPos) {
            if (currentLayerIndex < 0) {
                currentLayerIndex = 0;
                layerZs.push_back(currentPos.z);
                layers.emplace_back();
            }
            glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 0.5f, 1.0f));
            glm::vec3 dir = glm::normalize(currentPos - lastPos);
            float intensity = 0.8f + 0.4f * fabs(glm::dot(dir, lightDir));
            glm::vec3 finalColor = glm::clamp(currentColor * intensity,
                                             glm::vec3(0.0f), glm::vec3(1.0f));
            layers[currentLayerIndex].push_back({lastPos, finalColor});
            layers[currentLayerIndex].push_back({currentPos, finalColor});
        }

        if (!std::isnan(E)) {
            lastExtrusion = E;
        }
        lastPos = currentPos;
        hasLastPos = true;
    }
}
