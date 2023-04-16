#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) :
        cameraPosition{ cameraPosition },
        cameraTarget{ cameraTarget },
        cameraUpDirection{ cameraUp }, 
        cameraFrontDirection{cameraTarget - cameraPosition},
        cameraRightDirection{glm::cross(cameraFrontDirection, cameraUp)} {
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 dir;
        switch (direction) {
        case MOVE_FORWARD:
            dir = cameraFrontDirection * speed;
            break;
        case MOVE_BACKWARD:
            dir = cameraFrontDirection * (-speed);
            break;
        case MOVE_RIGHT:
            dir = cameraRightDirection * speed;
            break;
        case MOVE_LEFT:
            dir = cameraRightDirection * (-speed);
            break;
        default:
            break;
        }
        cameraPosition += dir;
        cameraTarget += dir;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw)
    {
        this->cameraYaw += yaw;
        this->cameraPitch += pitch;

        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        else if (pitch < -89.0f) {
            pitch = -89.0f;
        }

        glm::vec3 front(
            sin(glm::radians(this->cameraYaw) * glm::cos(glm::radians(this->cameraPitch))),
            sin(glm::radians(this->cameraPitch)),
            -cos(glm::radians(this->cameraYaw)) * glm::cos(glm::radians(this->cameraPitch))
        );

        this->cameraFrontDirection = glm::normalize(front);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
        this->cameraTarget = cameraPosition + cameraFrontDirection;
    }
}