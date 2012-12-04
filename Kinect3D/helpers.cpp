#include "helpers.h"

#include <Windows.h>
#include <gl/GL.h>
#include <NuiApi.h>

void render_points_colored( const std::vector<Point> & points ){
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Point), points.data());
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Point), &points[0].color);
    glDrawArrays(GL_POINTS, 0, points.size());
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void render_points( const std::vector<Point> & points ){
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Point), points.data());
    glDrawArrays(GL_POINTS, 0, points.size());
    glDisableClientState(GL_VERTEX_ARRAY);
}

void render_skeleton_points( const float * skeleton){
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(4, GL_FLOAT, 0, skeleton);
    glDrawArrays(GL_POINTS, 0, NUI_SKELETON_POSITION_COUNT);
    glDisableClientState(GL_VERTEX_ARRAY);
}

unsigned spine_list[] = {
NUI_SKELETON_POSITION_HIP_CENTER, 
NUI_SKELETON_POSITION_SPINE,
NUI_SKELETON_POSITION_SHOULDER_CENTER,
NUI_SKELETON_POSITION_HEAD
};

unsigned arm_list[] = {
    NUI_SKELETON_POSITION_HAND_LEFT,
    NUI_SKELETON_POSITION_WRIST_LEFT,
    NUI_SKELETON_POSITION_ELBOW_LEFT,
    NUI_SKELETON_POSITION_SHOULDER_LEFT,
    NUI_SKELETON_POSITION_SHOULDER_CENTER,
    NUI_SKELETON_POSITION_SHOULDER_RIGHT,
    NUI_SKELETON_POSITION_ELBOW_RIGHT,
    NUI_SKELETON_POSITION_WRIST_RIGHT,
    NUI_SKELETON_POSITION_HAND_RIGHT
};

unsigned leg_list[] = {
    NUI_SKELETON_POSITION_FOOT_LEFT,
    NUI_SKELETON_POSITION_ANKLE_LEFT,
    NUI_SKELETON_POSITION_KNEE_LEFT,
    NUI_SKELETON_POSITION_HIP_LEFT,
    NUI_SKELETON_POSITION_HIP_CENTER, 
    NUI_SKELETON_POSITION_HIP_RIGHT,
    NUI_SKELETON_POSITION_KNEE_RIGHT,
    NUI_SKELETON_POSITION_ANKLE_RIGHT,
    NUI_SKELETON_POSITION_FOOT_RIGHT
};

void render_skeleton( const float * skeleton){
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(4, GL_FLOAT, 0, skeleton);
    glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_INT, spine_list);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_INT, arm_list);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_INT, leg_list);
    glDisableClientState(GL_VERTEX_ARRAY);
}