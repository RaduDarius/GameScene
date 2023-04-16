#ifndef Camera_hpp
#define Camera_hpp

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include <string>
#include <iostream>

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT};
    
    class Camera
    {
    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        
        void printCoordinates() {
            std::cout << cameraPosition.x << ' ' << cameraPosition.y << ' ' << cameraPosition.z << '\n';
            std::cout << cameraTarget.x << ' ' << cameraTarget.y << ' ' << cameraTarget.z << '\n';
        }

        void moveCamera(const glm::vec3 point);

        void moveTarget(const glm::vec3 point);

        glm::vec3 getTargetPos() const {
            return this->cameraTarget;
        }

        glm::vec3 getCamPos() const {
            return this->cameraPosition;
        }

    private:
        float cameraPitch{ 0.0f };
        float cameraYaw{ 0.0f };

        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
    };
    
}

#endif /* Camera_hpp */
