#pragma once
#include "Header.h"

struct SQT
{
public:
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 positon = glm::vec3(0, 0, 0);
    float scale = 1.0f;
    float timeStamp = -1;
    const char* jointName;
};

struct AnimatedNode
{
public:
    AnimatedNode(const char* name) {
        m_nodeName = name;
    }
    std::vector<SQT> m_nodeKeys;
    const char* m_nodeName;
};


class Animation
{
public: // methods
    Animation(const char* Filename);
    ~Animation();
    float GetTicksPerSecond();

public: // fields                
    float m_duration;
    float m_ticksPerSecond;
    float m_finalTimeStamp;
    const char* m_filename;

    std::vector<AnimatedNode> m_animatedNodes;
    std::map<const char*, unsigned int> m_NodeMapping; // maps a node name to its index
};
