#pragma once
#include "Common.h"

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f;

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
	float fov;
    vec3 position;
    vec3 gaze;
    vec3 up;
    vec3 right;
    vec3 world_up;

    float yaw;
    float pitch;
    float mouse_sensitivity;
    

public:
	Camera(float fov,vec3 position,vec3 gaze,vec3 up);

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position, position + gaze, up);
    }


	void ProcessKeyboard(Camera_Movement direction, float &delta_time);
    void ProcessMouseScroll(float yoffset);
	void ProcessMouseMovement(float &xoffset, float& yoffse);
private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new gsze vector
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        gaze = glm::normalize(front);
        // also re-calculate the Right and Up vector
        right = glm::normalize(glm::cross(gaze, world_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up = glm::normalize(glm::cross(right, gaze));   
    }
};

Camera::Camera(float fov, vec3 position, vec3 gaze, vec3 up) 
    :fov(fov),position(position),gaze(gaze),up(up)
{
    mouse_sensitivity = SENSITIVITY;
    world_up = up;
    yaw = YAW;
    pitch = PITCH;
    updateCameraVectors();
}


void Camera::ProcessMouseMovement(float &xoffset, float &yoffset)
{
    GLboolean constrain_pitch = true;
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrain_pitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void Camera::ProcessKeyboard(Camera_Movement direction, float &deltaTime)
{
    float velocity = mouse_sensitivity * deltaTime;
    if (direction == FORWARD)
        position += gaze * velocity;
    if (direction == BACKWARD)
        position -= gaze * velocity;
    if (direction == LEFT)
        position -= right * velocity;
    if (direction == RIGHT)
        position += right * velocity;
    if (direction == UP)
        position += up * velocity;
    if (direction == DOWN)
        position -= up * velocity;
}