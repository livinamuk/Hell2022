#include "Editor.h"
#include "Core/Scene.h"
#include "Renderer/Renderer.h"
#include "Renderer/TextBlitter.h"
#include "earcut/earcut.hpp"
#include <array>
#include "Core/GameData.h"
#include "Core/File.h"

bool Editor::s_isOpen = false;
float Editor::s_zoom = 500;
float Editor::s_cameraX = 0;
float Editor::s_cameraZ = 0; 
float Editor::s_scrollSpeed = 0.1f;
float Editor::s_mouseX;
float Editor::s_mouseZ;
float Editor::s_gridX;
float Editor::s_gridZ;

std::vector<glm::vec3> Editor::s_newRoomVertices;

bool Editor::VertexEquality(glm::vec3 a, glm::vec3 b)
{
    return ((int)std::round(a.x * 10) == (int)std::round(b.x * 10) &&
        (int)std::round(a.y * 10) == (int)std::round(b.y * 10) &&
        (int)std::round(a.z * 10) == (int)std::round(b.z * 10));
}

glm::vec3* Editor::VertexExistsAtThisLocation(glm::vec3 query)
{
    // New room vertices
    for (glm::vec3& v : s_newRoomVertices)
        if (VertexEquality(v, query))
            return &v;

    // Room vertices
    for (Room& room : GameData::s_rooms) {
        for (glm::vec3& v : room.m_vertices)
            if (VertexEquality(v, query))
                return &v;
    }

    return nullptr;
}

float sign(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
    return (p1.x - p3.x) * (p2.z - p3.z) - (p2.x - p3.x) * (p1.z - p3.z);
}

bool PointInTriangle(glm::vec3 pt, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}


void Editor::CheckForHoveredDoor()
{
    for (Door& door : GameData::s_doors)
    {
        // v1      v3
        // 
        // v2      v4

        // ok so this doesn't work

        if (PointInTriangle(GetMousePosVec3(), door.GetVert1(), door.GetVert2(), door.GetVert3()) ||
            PointInTriangle(GetMousePosVec3(), door.GetVert3(), door.GetVert4(), door.GetVert1())) {
            s_hoveredDoor.door = &door;
            return;
        }
        // check if mouse cursor lies within door rectangle
    }
}

void Editor::CheckForHoveredLine()
{



    // Look over all rooms
    float minD = 0.1f;
    for (Room& room : GameData::s_rooms)
    {
        for (int i = 1; i < room.m_vertices.size(); i++)
        {
            glm::vec3& v1(room.m_vertices[i]);
            glm::vec3& v2(room.m_vertices[i - 1]);
            float d = Minimum_Distance(glm::vec2(v1.x, v1.z), glm::vec2(v2.x, v2.z), glm::vec2(s_gridX, s_gridZ));
            if (d < minD) {
                //Renderer::DrawLine(shader, v1, v2, color);
                s_hoveredLine.found = true;
                s_hoveredLine.room = &room;
                s_hoveredLine.vertIndex1 = i;
                s_hoveredLine.vertIndex2 = i-1;
                s_hoveredLine.v1 = &v1;
                s_hoveredLine.v2 = &v2;
                return;
            }
        }

        // The final line..
        glm::vec3& v1(room.m_vertices[0]);
        glm::vec3& v2(room.m_vertices[room.m_vertices.size() - 1]);
        float d = Minimum_Distance(glm::vec2(v1.x, v1.z), glm::vec2(v2.x, v2.z), glm::vec2(s_gridX, s_gridZ));
        if (d < minD) {
           // Renderer::DrawLine(shader, v1, v2, color);
            s_hoveredLine.found = true;
            s_hoveredLine.room = &room;
            s_hoveredLine.vertIndex1 = 0;
            s_hoveredLine.vertIndex2 = room.m_vertices.size() - 1;
            s_hoveredLine.v1 = &v1;
            s_hoveredLine.v2 = &v2;
            return;
        }
    }
}


glm::vec3 Editor::ClosestPointOnLine(glm::vec3 point, glm::vec3 start, glm::vec3 end)
{
    glm::vec2 p(point.x, point.z);
    glm::vec2 v(start.x, start.z);
    glm::vec2 w(end.x, end.z);

    const float l2 = ((v.x - w.x) * (v.x - w.x)) + ((v.y - w.y) * (v.y - w.y));
    if (l2 == 0.0)
        return glm::vec3(0); 

    const float t = std::max(0.0f, std::min(1.0f, dot(p - v, w - v) / l2));
    const glm::vec2 projection = v + t * (w - v);
    
    return glm::vec3(projection.x, 0, projection.y);
}

float Editor::Minimum_Distance(glm::vec2 v, glm::vec2 w, glm::vec2 p)
{
    // Return minimum distance between line segment vw and point p
    const float l2 = ((v.x - w.x)* (v.x - w.x)) + ((v.y - w.y)* (v.y - w.y));  // length_squared(v, w);
    if (l2 == 0.0) 
        return distance(p, v);   // v == w case

    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line. 
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.

    const float t = std::max(0.0f, std::min(1.0f, dot(p - v, w - v) / l2));
    const glm::vec2 projection = v + t * (w - v);  // Projection falls on the segment
    return distance(p, projection);
}


Action s_editorAction = Action::IDLE;
DragState s_dragState = DragState::IDLE;
SelectedState s_selectedState = SelectedState::IDLE;
void* s_draggedObject = nullptr;
void* s_selectedObject = nullptr;
int s_dragTime = 0;

glm::vec3 Editor::GetMouseGridPosVec3()
{
    return glm::vec3(s_gridX, 0, s_gridZ);
}

void Editor::RebuildAllMeshData()
{
    for (Room& room : GameData::s_rooms)
        room.BuildMeshFromVertices();
}

glm::vec3 Editor::GetMousePosVec3()
{
    return glm::vec3(s_mouseX, 0, s_mouseZ);
}

void Editor::Update(float screenWidth, float screenHeight)
{

    if (Input::s_keyDown[HELL_KEY_P])
        File::SaveMap("Map.json");

    if (Input::s_keyDown[HELL_KEY_N])
        GameData::Clear();

    if (Input::s_keyDown[HELL_KEY_W])
        s_cameraZ -= s_scrollSpeed;
    if (Input::s_keyDown[HELL_KEY_S])
        s_cameraZ += s_scrollSpeed;
    if (Input::s_keyDown[HELL_KEY_D])
        s_cameraX += s_scrollSpeed;
    if (Input::s_keyDown[HELL_KEY_A])
        s_cameraX -= s_scrollSpeed;

    s_zoom *= (1 + ((float)-Input::s_mouseWheelValue / 5));
    s_zoom = std::max(s_zoom, 70.0f);
    s_zoom = std::min(s_zoom, 2000.0f);
    Input::s_mouseWheelValue = 0;

    s_mouseX = (Input::s_mouseX - screenWidth / 2) / (s_zoom / 2) + s_cameraX;
    s_mouseZ = (Input::s_mouseY - screenHeight / 2) / (s_zoom / 2) + s_cameraZ;

    // Round mouse position to the nearest grid square
    s_gridX = (int)std::round(s_mouseX * 10) / 10.0f;
    s_gridZ = (int)std::round(s_mouseZ * 10) / 10.0f;

    // Drag timer
    s_dragTime++;


    // Hovered vertex
    s_hoveredVertex.found = false;
    s_hoveredVertex.position = nullptr;
    s_hoveredVertex.room = nullptr;
    for (Room& room : GameData::s_rooms) {
        for (glm::vec3& v : room.m_vertices) {
            if (VertexEquality(v, glm::vec3(s_gridX, 0, s_gridZ)))
            {
                s_hoveredVertex.found = true;
                s_hoveredVertex.position = &v;
                s_hoveredVertex.room = &room;
            }
        }
    }

    // glm::vec3* hoveredVert = VertexExistsAtThisLocation();

     // Hovered line

    //if (!s_hoveredVertex.found)
    


    // Reset all hovers
    s_hoveredDoor.door = nullptr;

    s_hoveredLine.found = false;
    s_hoveredLine.room = nullptr;
    s_hoveredLine.vertIndex1 = -1;
    s_hoveredLine.vertIndex2 = -1;
    s_hoveredLine.v1 = nullptr;
    s_hoveredLine.v2 = nullptr;

    // First check for hovered doors
    CheckForHoveredDoor();
    // If none was found then check for hovered lines
    if (!s_hoveredDoor.WasFound())
        CheckForHoveredLine();



    ///////////////////
    //               //
    //     Doors     //


    if (Input::KeyPressed(HELL_KEY_1) && s_editorAction == Action::IDLE && s_hoveredLine.found)
    {
        PlaceDoorAtMousePos();
        RebuildAllMeshData();
    }



    /*
     
     
    // Creating
    if (Input::KeyPressed(HELL_KEY_1) && s_editorAction == Action::IDLE)
    {
        ResetEditorState();
        s_editorAction = Action::PLACING_DOOR;
        s_selectedState = SelectedState::DOOR_SELECTED;

        GameData::s_doors.push_back(Door());
        s_selectedObject = &GameData::s_doors.back();
    }
    // Pin the selected door to the mouse cursor
    if (s_editorAction == Action::PLACING_DOOR)
        static_cast<Door*>(s_selectedObject)->m_transform.position = GetMousePosVec3();

    // Rotating
    if (s_editorAction == Action::PLACING_DOOR && Input::KeyDown(HELL_KEY_Q)) {
        static_cast<Door*>(s_selectedObject)->m_transform.rotation.y -= 0.1f;
    }
    if (s_editorAction == Action::PLACING_DOOR && Input::KeyDown(HELL_KEY_R)) {
        static_cast<Door*>(s_selectedObject)->m_transform.rotation.y += 0.1f;
    }


    // Place the Door
    if (s_editorAction == Action::PLACING_DOOR && Input::LeftMousePressed())
        ResetEditorState();

    // Delete the selected door
    if (s_editorAction == Action::PLACING_DOOR && Input::RightMousePressed()) 
    {
        for (int i = 0; i < GameData::s_doors.size(); i++)
            if (&GameData::s_doors[i] == s_selectedObject)
                GameData::s_doors.erase(GameData::s_doors.begin() + i);

        ResetEditorState();
    }*/



    // Selected (and drag vertex)
    if (Input::LeftMousePressed())
    {
        // reset
        s_selectedObject = nullptr;
        s_selectedState = SelectedState::IDLE;

        // vertex?
        if (VertexExistsAtThisLocation(glm::vec3(s_gridX, 0, s_gridZ))) {
            s_selectedState = SelectedState::VERTEX_SELECTED;
            s_selectedObject = VertexExistsAtThisLocation(glm::vec3(s_gridX, 0, s_gridZ));
        }
        // door?
        else if (s_hoveredDoor.WasFound()) {
            s_selectedState = SelectedState::DOOR_SELECTED;
            s_selectedObject = s_hoveredDoor.door;
        }




        // and begin dragging it
        if (s_dragState == DragState::IDLE) // this may be redundant. CLEAAAAAAAAAAAAAAAAAAN UP THIS SHIT.
        {          
            s_draggedObject = s_selectedObject;

            if (s_draggedObject != nullptr) {
                s_dragState = DragState::DRAGGING_VERTEX;
                s_dragTime = 0;
            }
        }
    }

    // Select a vertex
    /*if (Input::LeftMousePressed())
    {
        // Find the vertex is you're hovered on one
        if (s_dragState == DragState::IDLE && hoveredVert != nullptr)
        {
            for (Room& room : GameData::s_rooms)
            {
                for (int i = 0; i < room.m_vertices.size(); i++)
                {
                    if (&room.m_vertices[i] == hoveredVert)
                    {
                        s_selectedState = SelectedState::SELECTED_VERTEX;
                        s_selectedVertex.room = &room;
                        s_selectedVertex.index = i;

                    std:cout << "SECLTED A VERT\n";
                        // goto theEnd;
                    }
                }
            }
        }
        // theEnd:            
    }*/

    // Dragging vertex
    if (s_dragState == DragState::DRAGGING_VERTEX) {
        DragSelectedVertex();
    }

    // Delete a vertex
    if (s_selectedState == SelectedState::VERTEX_SELECTED && Input::KeyPressed(HELL_KEY_BACKSPACE)) {
        DeleteSelectedVertex();
    }

    // Flip wall normals
    if (s_editorAction == Action::IDLE && Input::KeyPressed(HELL_KEY_V) && s_hoveredLine.found)
    {
        s_hoveredLine.room->m_invertWallNormals = !s_hoveredLine.room->m_invertWallNormals;
        s_hoveredLine.room->BuildMeshFromVertices();
    }
    else if (Input::KeyPressed(HELL_KEY_V) && s_hoveredVertex.found)
    {
        s_hoveredVertex.room->m_invertWallNormals = !s_hoveredVertex.room->m_invertWallNormals;
        s_hoveredVertex.room->BuildMeshFromVertices();
    }


    // Place points
    if (Input::KeyPressed(HELL_KEY_SPACE) && !s_hoveredLine.found)
    {
        ResetSelectedAndDragStates();

        // Being creating new room new room
        if (s_editorAction == Action::IDLE) {
            s_newRoomVertices.clear();
            glm::vec3 point(s_gridX, 0, s_gridZ);
            s_newRoomVertices.emplace_back(point);
            s_editorAction = Action::CREATING_ROOM;
            Audio::PlayAudio("UI_Select3.wav");
        }        
        
        // Create new room
        else if (s_editorAction == Action::CREATING_ROOM) 
        {
            // Commit new room if it's the end
            if (s_newRoomVertices.size() >= 3 && VertexEquality(glm::vec3(s_gridX, 0, s_gridZ), s_newRoomVertices[0])) 
            {                
                GameData::s_rooms.push_back(Room());
                Room* room = &GameData::s_rooms[GameData::s_rooms.size() - 1];
                room->m_vertices = s_newRoomVertices;
                room->BuildMeshFromVertices();

                s_newRoomVertices.clear();


               // Editor::ReCalculateAllDoorPositions();
               // Editor::RebuildAllMeshData();

                s_editorAction = Action::IDLE;
                Audio::PlayAudio("UI_Select4.wav");
            }
            // Otherwise record the next point
            else {
                glm::vec3 point(s_gridX, 0, s_gridZ);
                s_newRoomVertices.emplace_back(point);
                Audio::PlayAudio("UI_Select3.wav");
            }

        }
    } 

    // Add vertex to hovered line
    if (Input::KeyPressed(HELL_KEY_SPACE) && s_hoveredLine.found && s_editorAction != Action::CREATING_ROOM)
    {
        Room* room = static_cast<Room*>(s_hoveredLine.room);

        glm::vec3 oldVert1Pos = room->m_vertices[s_hoveredLine.vertIndex1];
        glm::vec3 oldVert2Pos = room->m_vertices[s_hoveredLine.vertIndex2];

        room->m_vertices.insert(room->m_vertices.begin() + s_hoveredLine.vertIndex1, glm::vec3(s_gridX, 0, s_gridZ));
        room->BuildMeshFromVertices();

        Audio::PlayAudio("UI_Select3.wav"); 


        // Check if any doors need their parent vertex indices shifted
        for (Door& door : GameData::s_doors) 
        {
            Room* parentRoom = &GameData::s_rooms[door.m_parentRoomIndex];
            if (parentRoom == s_hoveredLine.room)
            {
                float distFromDoor = glm::distance(GetMouseGridPosVec3(), *s_hoveredLine.v1);

                // find point of line of DOOR POS to A
                glm::vec3 closestPointOnDoorToVert1LineSegment = ClosestPointOnLine(GetMouseGridPosVec3(), door.m_transform.position, oldVert1Pos);
                glm::vec3 closestPointOnDoorToVert2LineSegment = ClosestPointOnLine(GetMouseGridPosVec3(), door.m_transform.position, oldVert2Pos);

                float distA = glm::distance(GetMouseGridPosVec3(), closestPointOnDoorToVert1LineSegment);
                float distB = glm::distance(GetMouseGridPosVec3(), closestPointOnDoorToVert2LineSegment);

                // if the mouse is to the LEFT? of the door
                if (distA < distB) {

                    // first accomidate for edge case when the wall is made up of verts 0 and size-1
                    if (s_hoveredLine.vertIndex1 == 0)
                        door.m_parentIndexVertexB++;
                    // now check as normal
                    else if (s_hoveredLine.vertIndex1 < door.m_parentIndexVertexA) {
                        door.m_parentIndexVertexA++;
                        door.m_parentIndexVertexB++;
                    }
                    std::cout << "closer to A " << distA << " " << distB << "\n";
                }
                else {
                    // first accomidate for edge case when the wall is made up of verts 0 and size-1
                    if (s_hoveredLine.vertIndex1 == 0) {
                        door.m_parentIndexVertexA = 0;
                        door.m_parentIndexVertexB = 1;
                      // 
                    }
                    // now check as normal
                    else if (s_hoveredLine.vertIndex1 <= door.m_parentIndexVertexA) {
                        door.m_parentIndexVertexA++;
                        door.m_parentIndexVertexB++;

                        std::cout << "closer to B " << distA << " " << distB << "\n";
                     //   closer to B 2.50129 0.0253497
                     //   when to the right
                        //closer to A 0.0408409 0.902243
                    }
                }
            }
        }

        


        ResetSelectedAndDragStates();
    }
        
    if (Input::RightMousePressed())
    {
        // Cancel room creation
        if (s_editorAction == Action::CREATING_ROOM) {
            s_newRoomVertices.clear();
            s_editorAction = Action::IDLE;
        }
    }
}

void Editor::Render(float screenWidth, float screenHeight)
{
    // Timer and color bull shit
    static float timer = 0;
    timer += 0.15f;
    float flash = 0;
    if ((int)timer % 2)
        flash = 1;

    /*std::cout << s_newRoomVertices.size() << "\n";

    if (GameData::s_rooms.size() > 0)
        std::cout << "room[0]: " << GameData::s_rooms[0].m_vertices.size() << "\n";*/

    Editor::Update(screenWidth, screenHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glm::vec3 viewPos = glm::vec3(s_cameraX, 3, s_cameraZ);
    float width = (float)screenWidth / s_zoom;
    float height = (float)screenHeight / s_zoom;

    Renderer::DrawGrid(viewPos, false);

    // glBindFramebuffer(GL_FRAMEBUFFER, renderer->m_gBuffer.ID);

    glm::mat4 view = glm::lookAt(viewPos, viewPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
    glm::mat4 proj = glm::ortho(-width, width, -height, height, NEAR_PLANE, FAR_PLANE);

    glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glPointSize(13);
    glDisable(GL_DEPTH_TEST);

    Shader* shader = &Renderer::s_geometry_shader;
    shader->use();



    // Draw rooms
    for (auto& room : GameData::s_rooms) {
        room.DrawFloor(shader);
    }

    // Draw static entities
    for (EntityStatic entityStatic : GameData::s_staticEntities)
        entityStatic.DrawEntity(shader);


    ///
    /// S O L I D   C O L O R
    ///
 
    shader = &Renderer::s_solid_color_shader_editor;
    shader->use();
    shader->setMat4("proj", proj);
    shader->setMat4("view", view);



    glDisable(GL_DEPTH_TEST);
    glLineWidth(3);

    // Render the vertices and lines for all existing rooms 
    for (Room& room : GameData::s_rooms)
    {
        // Temporarily re-add the first vertex to the end (removed after for loop)
        room.m_vertices.push_back(room.m_vertices[0]);

        for (int i = 1; i < room.m_vertices.size(); i++)
        {
            glm::vec3 v1(room.m_vertices[i]);
            glm::vec3 v2(room.m_vertices[i - 1]);

            Renderer::DrawPoint(shader, v1, glm::vec3(1, 0, 0));
            Renderer::DrawPoint(shader, v2, glm::vec3(1, 0, 0));
            Renderer::DrawLine(shader, v1, v2, glm::vec3(1, 1, 1));

            // Draw wall normals
            glm::vec3 wallVector = glm::normalize(v2 - v1);
            glm::vec3 wallNormal = glm::vec3(-wallVector.z, 0, wallVector.x);
            if (room.m_invertWallNormals) 
                wallNormal *= glm::vec3(-1);
            glm::vec3 midPoint = (v1 + v2) * glm::vec3(0.5);
            glm::vec3 normalEndPoint = midPoint + (wallNormal * glm::vec3(0.1f));
            Renderer::DrawLine(shader, midPoint, normalEndPoint, glm::vec3(1, 1, 1));

        }
        // remove that temporary final element of the vertices vector that u added
        room.m_vertices.pop_back();
    }


    // Render the lines and vertices any room in creation
    {
        // Draw the vertices the room currently being created (if one is so)
        for (glm::vec3& point : s_newRoomVertices) {
            Renderer::DrawPoint(shader, glm::vec3(point.x, 1, point.z), glm::vec3(1, 0, 0));
        }

        // Draw the line extending from the last new room vertices to the mouse location
        if (s_newRoomVertices.size() > 0)
        {
            int i = s_newRoomVertices.size() - 1;
            glm::vec3 mousePoint = glm::vec3(s_gridX, 0, s_gridZ);
            Renderer::DrawPoint(shader, mousePoint, glm::vec3(1, 0, 0));
            Renderer::DrawLine(shader, s_newRoomVertices[i], mousePoint, glm::vec3(1, 1, 1));
        }

        // Draw the lines of the currently in creation room
        if (s_newRoomVertices.size() >= 2)
        {
            for (int i = 1; i < s_newRoomVertices.size(); i++)
            {
                glm::vec3 v1(s_newRoomVertices[i]);
                glm::vec3 v2(s_newRoomVertices[i - 1]);

                Renderer::DrawPoint(shader, v1, glm::vec3(1, 0, 0));
                Renderer::DrawPoint(shader, v2, glm::vec3(1, 0, 0));
                Renderer::DrawLine(shader, v1, v2, glm::vec3(1, 1, 1));
            }
        }
    }

    if (s_hoveredLine.found && !s_hoveredVertex.found) 
    {
        // draw line
        Renderer::DrawLine(shader, *s_hoveredLine.v1, *s_hoveredLine.v2, glm::vec3(flash));
      //  Renderer::DrawLine(shader, *s_hoveredLine.v1, *s_hoveredLine.v2, glm::vec3(1, 1, 0));
    }
        
    

    // are you on the end point???
    //if (s_newRoomVertices.size() >= 4 && VertexEquality(glm::vec3(s_gridX, 0, s_gridZ), s_newRoomVertices[0]))
    //    Renderer::DrawPoint(shader, s_newRoomVertices[0], glm::vec3(0, 1, 0));
    


    // Draw and flash the selected vertex
    if (s_selectedState == SelectedState::VERTEX_SELECTED) 
    {
        glm::vec3* vert = static_cast<glm::vec3*>(s_selectedObject);
        Renderer::DrawPoint(shader, *vert, glm::vec3(flash, flash, 0));
    }

    // Draw the hovered vert solid
    glm::vec3* existingVert = VertexExistsAtThisLocation(glm::vec3(s_gridX, 0, s_gridZ));
    if (existingVert != nullptr)
        Renderer::DrawPoint(shader, *existingVert, glm::vec3(1, 1, 0));

    // Draw doors
    for (Door& door : GameData::s_doors) {
        if (&door == s_hoveredDoor.door)
            door.DrawForEditor(shader, glm::vec3(1, 1, 0));
        else if (&door == s_selectedObject)
            door.DrawForEditor(shader, glm::vec3(flash, flash, 0));
        else
            door.DrawForEditor(shader, glm::vec3(0));
    }
    



    //glm::vec3 p = ClosestPointOnLine(GetMouseGridPosVec3(), GameData::s_rooms[0].m_vertices[0], GameData::s_rooms[0].m_vertices[1]);

  //  Renderer::DrawPoint(shader, p, glm::vec3(0, 1, 1));



  /*


    glLineWidth(6);
 Line lineA;
    lineA.start.x = GameData::s_doors[0].GetVert1().x;
    lineA.start.z = GameData::s_doors[0].GetVert1().z;
    lineA.end.x = GameData::s_doors[0].GetVert2().x;
    lineA.end.z = GameData::s_doors[0].GetVert2().z;

    Line lineB;
    lineB.start.x = -2.6;
    lineB.start.z = -1.7;
    lineB.end.x = s_mouseX;
    lineB.end.z = s_mouseZ;


    float x = -1;
    float z = -1;

    glm::vec2 begin_a(lineA.start.x, lineA.start.z);
    glm::vec2 end_a(lineA.end.x, lineA.end.z);

    glm::vec2 begin_b(lineB.start.x, lineB.start.z);
    glm::vec2 end_b(lineB.end.x, lineB.end.z);
    
    glm::vec2 result(0, 0);

    int test = Util::LineIntersects(begin_a, end_a, begin_b, end_b, &result);

    glm::vec3 color = glm::vec3(1, 1, 0);

    if (test == DO_INTERSECT)
        color = glm::vec3(1, 0, 1);


    Renderer::DrawLine(shader, glm::vec3(lineA.start.x, 0, lineA.start.z), glm::vec3(lineA.end.x, 0, lineA.end.z), glm::vec3(1, 1, 0));
    Renderer::DrawLine(shader, glm::vec3(lineB.start.x, 0, lineB.start.z), glm::vec3(lineB.end.x, 0, lineB.end.z), color);

    Renderer::DrawPoint(shader, glm::vec3(result.x, 0, result.y), glm::vec3(1, 1, 1));
    */

    glLineWidth(1);



    SubmitBlitterText();
    Renderer::TextBlitterPass(&Renderer::s_textued_2D_quad_shader);

    glLineWidth(1);





    }








void Editor::SubmitBlitterText()
{
    TextBlitter::BlitLine("Mouse pos: " + Util::FloatToString(s_mouseX, 3) + ", " + Util::FloatToString(s_mouseZ, 3));
    TextBlitter::BlitLine("Grid Pos:  " + Util::FloatToString(s_gridX, 1) + ", " + Util::FloatToString(s_gridZ, 1));
    TextBlitter::BlitLine(" ");
   // TextBlitter::BlitLine("New room verts: " + std::to_string(Editor::s_newRoom.m_vertices.size()));

   /* if (GameData::s_rooms.size() > 0)
        TextBlitter::BlitLine("Sum of edges: " + std::to_string(GameData::s_rooms[0].m_sumOfEdges));

    if (s_hoveredLine.found) {
        TextBlitter::BlitLine("Hovered vert A: " + std::to_string(s_hoveredLine.vertIndex1));
        TextBlitter::BlitLine("Hovered vert B: " + std::to_string(s_hoveredLine.vertIndex2));
    }

    if (GameData::s_doors.size() > 0) {
        TextBlitter::BlitLine("Door vert A: " + std::to_string(GameData::s_doors[0].m_parentIndexVertexA));
        TextBlitter::BlitLine("Door vert B: " + std::to_string(GameData::s_doors[0].m_parentIndexVertexB));
    }*/

   /* if (s_hoveredDoor.WasFound())
    {
        Door* door = s_hoveredDoor.door;

        PxQuat q = door->m_rigid->getGlobalPose().q;
        //glm::quat rotation(q.w, q.x, q.y, q.z);

        std::string str = "Door rot: ";
        str += std::to_string(q.w) + " ";
        str += std::to_string(q.x) + " ";
        str += std::to_string(q.y) + " ";
        str += std::to_string(q.z); 
        TextBlitter::BlitLine(str);
    }*/


  /*  if (GameData::s_rooms.size() > 0)
    {

        TextBlitter::BlitLine("VERT COUNT: " + std::to_string(GameData::s_rooms[0].m_vertices.size()));

        for (int i = 0; i < GameData::s_rooms[0].m_walls.size(); i++) {
            TextBlitter::BlitLine("wall vert count: " + std::to_string(i) + " " + std::to_string(GameData::s_rooms[0].m_walls[i].m_vertices.size()) + " " + std::to_string(GameData::s_rooms[0].m_walls[i].m_VAO));

            auto v1 = GameData::s_rooms[0].m_walls[i].m_vertices[0];
            auto v2 = GameData::s_rooms[0].m_walls[i].m_vertices[1];
            auto v3 = GameData::s_rooms[0].m_walls[i].m_vertices[2];
            auto v4 = GameData::s_rooms[0].m_walls[i].m_vertices[3];

            TextBlitter::BlitLine(" -" + std::to_string(v1.Position.x) + " " + std::to_string(v1.Position.y) + " " + std::to_string(v1.Position.z));
            TextBlitter::BlitLine(" -" + std::to_string(v2.Position.x) + " " + std::to_string(v2.Position.y) + " " + std::to_string(v2.Position.z));
            TextBlitter::BlitLine(" -" + std::to_string(v3.Position.x) + " " + std::to_string(v3.Position.y) + " " + std::to_string(v3.Position.z));
            TextBlitter::BlitLine(" -" + std::to_string(v4.Position.x) + " " + std::to_string(v4.Position.y) + " " + std::to_string(v4.Position.z));

        }

    }*/

}

void Editor::ToggleEditor()
{
	s_isOpen = !s_isOpen;
}

bool Editor::IsOpen()
{
	return s_isOpen;
}

void Editor::ResetSelectedAndDragStates()
{
    s_selectedState = SelectedState::IDLE;
    s_dragState = DragState::IDLE;
    s_selectedObject = nullptr;
    s_draggedObject = nullptr;
}

void Editor::DragSelectedVertex()
{
    // If user releases left mouse, then release the vertex
    if (!Input::LeftMouseDown()) {
        s_dragState = DragState::IDLE;

        //  if (s_dragTime > 5)
        //      ResetSelectedAndDragStates();

        return;
    }


    /*
    // Now first check this new vertex position doesn't CRUSH a door
    for (Door& door : GameData::s_doors)
    {
        if (door.GetPointerToParentVert1() == s_draggedObject)
        {
            // don't let the point within 0.6 of the door
            if (glm::distance(door.m_transform.position, GetMousePosVec3()) < 0.6)
                return;
            // don't let the point to jump PAST that on the other side
            if (glm::distance(GetMousePosVec3(), door.GetParentVert2()) < glm::distance(door.m_transform.position, door.GetParentVert2()))
                return;
            // don't let the point skip the check above by being further away than the whole wall
            if (glm::distance(GetMousePosVec3(), door.GetParentVert1()) >= glm::distance(door.GetParentVert1(), door.GetParentVert2()))
                return;
        }
        else if (door.GetPointerToParentVert2() == s_draggedObject)
        {
            // don't let the point within 0.6 of the door
            if (glm::distance(door.m_transform.position, GetMousePosVec3()) < 0.6)
                return;
            // don't let the point to jump PAST that on the other side
            if (glm::distance(GetMousePosVec3(), door.GetParentVert1()) < glm::distance(door.m_transform.position, door.GetParentVert1()))
                return;
            // don't let the point skip the check above by being further away than the whole wall
            if (glm::distance(GetMousePosVec3(), door.GetParentVert2()) >= glm::distance(door.GetParentVert2(), door.GetParentVert1()))
                return;
        }
    }*/

    // are you dragging a door? then don't let it within 0.6 of a vertex
   /* for (Door& door : GameData::s_doors)
    {
        if (s_selectedObject == &door)
        {
            if (glm::distance(door.m_transform.position, door.GetParentVert1()) < 0.6f ||
                glm::distance(door.m_transform.position, door.GetParentVert2()) < 0.6f)
                return;
        }
    }*/

    static_cast<glm::vec3*>(s_draggedObject)->x = s_gridX;
    static_cast<glm::vec3*>(s_draggedObject)->z = s_gridZ;

    // Remove any doors that got CRUSHED
    for (vector<Door>::iterator it = GameData::s_doors.begin();  it != GameData::s_doors.end();)
    {
        if (glm::distance(it->m_transform.position, it->GetParentVert1()) <= 0.6f ||
            glm::distance(it->m_transform.position, it->GetParentVert2()) <= 0.6f) 
        {
            it->RemoveCollisionObject();
            it = GameData::s_doors.erase(it);

            if (s_selectedState == SelectedState::DOOR_SELECTED)
                ResetSelectedAndDragStates();
        }
        else
            ++it;
    }


   /* for (int i = 0; i < GameData::s_doors.size(); i++)
    {
        Door* door = &GameData::s_doors[i];
        if (glm::distance(door->m_transform.position, door->GetParentVert1()) <= 0.6f ||
            glm::distance(door->m_transform.position, door->GetParentVert2()) <= 0.6f)
        {
            GameData::s_doors.erase(GameData::s_doors.begin() + i);
            i--;
        }
    }*/

    //for (Room& room : GameData::s_rooms)
    //    room.BuildMeshFromVertices();


    ReCalculateAllDoorPositions();
    RebuildAllMeshData();

    // ok u should only do a lot of this shit above for the current room. FIX LATER.
}

void Editor::DeleteSelectedVertex()
{
    // Find the vertex
    for (int r = 0; r < GameData::s_rooms.size(); r++)
    {
        Room& room = GameData::s_rooms[r];

        for (int v = 0; v < room.m_vertices.size(); v++)
        {
            if (&room.m_vertices[v] == s_selectedObject)
            {
                Audio::PlayAudio("UI_Select3.wav");

                // If room now has less than 3 vertices, remove the entire room
                if (room.m_vertices.size() <= 3)
                {
                    room.DeleteAllData();
                    GameData::s_rooms.erase(GameData::s_rooms.begin() + r);
                    s_selectedState = SelectedState::IDLE;
                    return;
                }            
                // Otherwise remove the single vertex
                else
                {
                    // Delete the vertex
                    room.m_vertices.erase(room.m_vertices.begin() + v);
                    room.BuildMeshFromVertices();
                    s_selectedState = SelectedState::IDLE;

                    // Iterate the doors, there may be work to do...
                    for (auto d = 0; d < GameData::s_doors.size(); d++)
                    {
                        Door* door = &GameData::s_doors[d];

                        // Check if deleting this vertex needs to remove this door
                        if (door->m_parentIndexVertexA == v || door->m_parentIndexVertexB == v) {
                            GameData::s_doors.erase(GameData::s_doors.begin() + d);
                            d--;
                        }

                        // Check if any doors need their parent vertex indices shifted
                        Room* parentRoom = &GameData::s_rooms[door->m_parentRoomIndex];
                        if (parentRoom == &GameData::s_rooms[r])
                        {
                            if (v < door->m_parentIndexVertexA)
                                door->m_parentIndexVertexA--;
                            if (v < door->m_parentIndexVertexB)
                                door->m_parentIndexVertexB--;
                        }
                    }


                    // First check if any doors depend on this vertex
                    Door* dependantDoor = nullptr;
                    glm::vec3 selectedVertexPosition = *static_cast<glm::vec3*>(s_selectedObject);

               /*     for (Door& door : GameData::s_doors)
                    {
                        if (door.m_parentVertexA == s_selectedObject) {
                            door.m_parentVertexA = nullptr;
                            dependantDoor = &door;
                        }
                        if (door.m_parentVertexB == s_selectedObject) {
                            door.m_parentVertexB = nullptr;
                            dependantDoor = &door;
                        }
                    }

                    // Delete the vertex
                    room.m_vertices.erase(room.m_vertices.begin() + v);
                    room.BuildMeshFromVertices();
                    s_selectedState = SelectedState::IDLE;

                    // If a door did depend on this. Replace the pointer with the vertex that took it's place.
                    if (dependantDoor != nullptr)
                    {
                        for (glm::vec3& vert : room.m_vertices)
                        {
                            if (VertexEquality(vert, selectedVertexPosition) && dependantDoor->m_parentVertexA == nullptr)
                                dependantDoor->m_parentVertexA = &vert;
                            if (VertexEquality(vert, selectedVertexPosition) && dependantDoor->m_parentVertexB == nullptr)
                                dependantDoor->m_parentVertexB = &vert;
                        }
                    }

                    return;*/
                }
            }
        }
    }
    
}

void Editor::DragSelectedDoor()
{
}

void Editor::ResetEditorState()
{
    // selected vertex
    s_selectedVertex.room = nullptr;
    s_selectedVertex.index = -1;

    // hovered line
    s_hoveredLine.found = false;
    s_hoveredLine.room = nullptr;
    s_hoveredLine.vertIndex1 = -1;
    s_hoveredLine.vertIndex2 = -1;
    s_hoveredLine.v1 = nullptr;
    s_hoveredLine.v2 = nullptr;

    // hovered vertex
    s_hoveredVertex.found = false;
    s_hoveredVertex.room = nullptr;
    s_hoveredVertex.position = nullptr;

    // Reset states
    s_editorAction = Action::IDLE;
    s_dragState = DragState::IDLE;
    s_selectedState = SelectedState::IDLE;

    // there is some stuff still missing from here. use with care...
}

void Editor::ReCalculateAllDoorPositions()
{
    for (Door& door : GameData::s_doors)
    {
       Room* parentRoom = &GameData::s_rooms[door.m_parentRoomIndex];

       glm::vec3* v1 = &parentRoom->m_vertices[door.m_parentIndexVertexA];
       glm::vec3* v2 = &parentRoom->m_vertices[door.m_parentIndexVertexB];

  //     std::cout << "indexA: " << door.m_parentIndexVertexA << "\n";
     //  std::cout << "indexB: " << door.m_parentIndexVertexB << "\n";

    /*   std::cout << "index: " << door.m_indexOfVertexA << "\n";

       v1 = room->m_vertices[door.m_indexOfVertexA];

       if (door.m_indexOfVertexA == room->m_vertices.size() -1)
           v2 = room->m_vertices[0];
       else
           v2 = room->m_vertices[door.m_indexOfVertexA + 1];

       */

       // IF you are dragging one of the doors vertices, then recalculate it's position based off the one that aint moving


       if (s_selectedObject == v1)
       {
           float dist = glm::distance(*v2, door.m_transform.position);
           glm::vec3 dir = glm::normalize(*v2 - *v1);
           door.m_transform.position = *v2 + dir * glm::vec3(-dist);
       }
       else
       {
           float dist = glm::distance(*v1, door.m_transform.position);
           glm::vec3 dir = glm::normalize(*v2 - *v1);
           door.m_transform.position = *v1 + dir * glm::vec3(dist);
       }

       float delta_x = v2->x - v1->x;
       float delta_y = v2->z - v1->z;
       float theta_radians = atan2(delta_y, delta_x);

       door.m_transform.rotation.y = -theta_radians;

       
       if (door.m_rigid != nullptr)
           door.RemoveCollisionObject();

       door.CreateCollisionObject();
       
    }
}

void Editor::PlaceDoorAtMousePos()
{
    // First check the door isn't too close to one of the walls vertices
    if (glm::distance(GetMouseGridPosVec3(), s_hoveredLine.GetVert2()) < 0.6)
        return;
    if (glm::distance(GetMouseGridPosVec3(), s_hoveredLine.GetVert1()) < 0.6)
        return;

    // Now check it aint too close to another door
    for (Door& door : GameData::s_doors)
        if (glm::distance(GetMouseGridPosVec3(), door.m_transform.position) < 1.0)
            return;

    Audio::PlayAudio("UI_Select3.wav");

    // Place a door
    for (Room& room : GameData::s_rooms)
        for (int i = 0; i < room.m_vertices.size(); i++)
            if (&room.m_vertices[i] == s_hoveredLine.v1)
            {
                GameData::s_doors.push_back(Door());
                Door* door = &GameData::s_doors.back();

                // Find rookm index
                for (int r = 0; r < GameData::s_rooms.size(); r++) {
                    if (&GameData::s_rooms[r] == s_hoveredLine.room)
                        door->m_parentRoomIndex = r;
                }

                // Now set the other shit
                door->m_transform.position = GetClosestPointFromMouseToHoveredLine();
                door->m_parentIndexVertexA = s_hoveredLine.vertIndex1;
                door->m_parentIndexVertexB = s_hoveredLine.vertIndex2;
            }

    ResetSelectedAndDragStates();

    ReCalculateAllDoorPositions();
    RebuildAllMeshData();
}

glm::vec3 Editor::GetClosestPointFromMouseToHoveredLine()
{
    return ClosestPointOnLine(GetMouseGridPosVec3(), *s_hoveredLine.v1, *s_hoveredLine.v2);
}
