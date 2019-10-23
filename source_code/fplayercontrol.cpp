#include "includes.h"
#define _USE_MATH_DEFINES
#include <Math.h>

fplayercontrol::fplayercontrol(fphysicscontroller* physics, fscene* scene, bool fly, float eyeheight, PxUserControllerHitReport* callback)
{
	//---- INITIALIZING DATA ----
	this->physics = physics;
	this->scene = scene;
	this->camera = &scene->get_camera();
	this->fly = fly;
	this->eyeheight = eyeheight;
	this->additionalCallback = callback;
	//this->characterInstance = &scene->mModels[scene->characterInstance];
	look_into_direction(-camera->z_axis());

	//----- CREATING CHARACTER CONTROLLER -----
	glm::vec3 camPos = camera->translation();
	PxCapsuleControllerDesc desc;
	desc.height = eyeheight * 3.0f / 4.0f;
	desc.radius = eyeheight / 4.0f;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.position = PxExtendedVec3(camPos.x, camPos.y - eyeheight * 1.5f / 4.0f, camPos.z);
	desc.stepOffset = 0.1f;
	desc.upDirection = PxVec3(0, 1, 0);
	desc.nonWalkableMode = PxControllerNonWalkableMode::ePREVENT_CLIMBING;
	desc.slopeLimit = cosf(glm::radians(10.0f));
	desc.material = physics->mPhysics->createMaterial(1, 1, 0);
	desc.reportCallback = this;
	cameraController = physics->mControllerManager->createController(desc);
}

//should be called via fixedupdate (after physics update)
void fplayercontrol::pre_px_update(float deltaT)
{
	//TODO
}

void fplayercontrol::post_px_update(float deltaT) {
	
	//Check Mouse Interactions (only if no mirror is focused)
	if (movingMirror == -1) {
		double x = cgb::input().cursor_position().x;
		double y = cgb::input().cursor_position().y;
		if (isnan(lastCursorX)) {
			lastCursorX = x;
			lastCursorY = y;
		}
		double deltaX = lastCursorX - x;
		double deltaY = lastCursorY - y;
		lastCursorX = x;
		lastCursorY = y;
		horizontalAngle += deltaX * MOUSE_SENTIVITY;
		verticalAngle = glm::max(glm::min(verticalAngle + deltaY * 0.005, M_PI / 2 - 0.01), -M_PI / 2 + 0.01);
	}

	//Increase jump-time
	if (jump >= 0) {
		jump += deltaT;
	}

	//Calculate moving Direction
	glm::vec3 moveDir = glm::vec3(0);
	glm::vec3 straight = -camera->z_axis();
	if (!fly) {
		straight.y = 0;
	}
	
	if (cgb::input().key_down(cgb::key_code::w)) {
		moveDir += straight;
	}
	if (cgb::input().key_down(cgb::key_code::s)) {
		moveDir -= straight;
	}
	if (cgb::input().key_down(cgb::key_code::d)) {
		moveDir += camera->x_axis();
	}
	if (cgb::input().key_down(cgb::key_code::a)) {
		moveDir -= camera->x_axis();
	}
	if (fly && cgb::input().key_down(cgb::key_code::e)) {
		moveDir += camera->y_axis();
	}
	if (fly && cgb::input().key_down(cgb::key_code::q)) {
		moveDir -= camera->y_axis();
	}
	if (jumpallowed && cgb::input().key_down(cgb::key_code::space) && jump == -1) {
		jump = deltaT;
		jumpystart = cameraController->getFootPosition().y;
		jumpallowed = false;
	}
	moveDir = ((length(moveDir) <= 0.01f) ? glm::vec3(0) : WALK_SPEED * normalize(moveDir));
	if (!fly) {
		if (jump == -1) {
			moveDir += glm::vec3(0, -0.2f, 0);	//gravity
		}
		else {
			float y = glm::max(2.0f - 2.0f * (2 * jump - 1) * (2 * jump - 1), 0.0f) + jumpystart;
			moveDir.y += y - (cameraController->getFootPosition().y);
			if (jump >= 1) {
				jump = -1;
			}
		}
	}

	//---- MOVE CAMERA CONTROLLER ----
	cameraController->move(PxVec3(moveDir.x, moveDir.y, moveDir.z), 0.001f, deltaT, PxControllerFilters());
	PxExtendedVec3 camPos = cameraController->getPosition();
	camera->set_translation(glm::vec3(camPos.x, camPos.y + eyeheight * 1.5f / 4.0f, camPos.z));
	glm::vec3 lookDir = glm::vec3(sin(horizontalAngle) * cos(verticalAngle), sin(verticalAngle), cos(horizontalAngle) * cos(verticalAngle));
	camera->set_rotation(glm::quatLookAt(lookDir, glm::vec3(0,1,0)));

	//characterInstance->transform[0] = glm::vec3(cos(horizontalAngle), 0, -sin(horizontalAngle));
	//characterInstance->transform[1] = glm::vec3(-sin(horizontalAngle), 0, -cos(horizontalAngle));
	//characterInstance->transform[3] = glm::vec3(camPos.x, camPos.y, camPos.z);
	//characterInstance->lastInstanceUpdate = updateTime;
	//scene->instancesUpdated = updateTime;
}

void fplayercontrol::look_into_direction(const glm::vec3& direction)
{
	verticalAngle = asin(direction.y);
	glm::vec2 projected = glm::normalize(glm::vec2(direction.x, direction.z));
	horizontalAngle = acos(projected.y) * ((direction.x > 0) - (direction.x < 0));	//this last thing is the signum of direction.x
}

void fplayercontrol::beam_along(const glm::vec3& direction)
{
	PxExtendedVec3 camPos = cameraController->getPosition() + PxExtendedVec3(direction.x, direction.y, direction.z);
	cameraController->setPosition(PxExtendedVec3(camPos.x, camPos.y, camPos.z));
	camera->set_translation(glm::vec3(camPos.x, camPos.y + eyeheight * 1.5f / 4.0f, camPos.z));
}

void fplayercontrol::update_position()
{
	glm::vec3 camPos = camera->translation();
	cameraController->setPosition(PxExtendedVec3(camPos.x, camPos.y - eyeheight * 1.5f / 4.0f, camPos.z));
	look_into_direction(-camera->z_axis());
}

void fplayercontrol::add_mirror(std::vector<PxRigidStatic*> actors, float maxDistance)
{
	this->mirrorActors.push_back(actors);
	std::vector<PxTransform> transforms;
	for (size_t i = 0; i < actors.size(); ++i) {
		transforms.push_back(actors[i]->getGlobalPose());
	}
	this->mirrorOriginalTransformations.push_back(transforms);
	this->mirrorMaxDistances.push_back(maxDistance);
}

void fplayercontrol::reset_mirrors() {
	for (size_t i = 0; i < mirrorActors.size(); ++i) {
		for (size_t j = 0; j < mirrorActors[i].size(); ++j) {
			mirrorActors[i][j]->setGlobalPose(mirrorOriginalTransformations[i][j]);
		}
	}
}

void fplayercontrol::set_final_region(PxRigidStatic* finalRegion)
{
	this->finalRegion = finalRegion;
}

bool fplayercontrol::on_final_region()
{
	PxVec3 finMin = finalRegion->getWorldBounds().minimum;
	PxVec3 finMax = finalRegion->getWorldBounds().maximum;
	PxExtendedVec3 camPos = cameraController->getPosition();
	return camPos.x > finMin.x&& camPos.x < finMax.x && camPos.z > finMin.z&& camPos.z < finMax.z;
}

bool fplayercontrol::fell_down()
{
	return cameraController->getPosition().y < -20;
}

void fplayercontrol::cleanup()
{
	cameraController->release();
}

void fplayercontrol::onShapeHit(const PxControllerShapeHit& hit)
{
	if (!fly && hit.actor->getWorldBounds().maximum.y - cameraController->getPosition().y < 0.1) {
		jumpallowed = true;
	}
	if (additionalCallback != nullptr) {
		additionalCallback->onShapeHit(hit);
	}
}

void fplayercontrol::onControllerHit(const PxControllersHit& hit)
{
	if (additionalCallback != nullptr) {
		additionalCallback->onControllerHit(hit);
	}
}

void fplayercontrol::onObstacleHit(const PxControllerObstacleHit& hit)
{
	if (additionalCallback != nullptr) {
		additionalCallback->onObstacleHit(hit);
	}
}

int fplayercontrol::raycastMirror()
{
	glm::vec3 camPos = camera->translation();
	glm::vec3 camDir = -camera->z_axis();
	PxRaycastBuffer hit;
	physics->mPxScene->raycast(PxVec3(camPos.x + camDir.x * 0.45, camPos.y + camDir.y * 0.45, camPos.z + camDir.z * 0.45), PxVec3(camDir.x, camDir.y, camDir.z), 50, hit);
	//If looking at a mirror -> set as current moving mirror
	if (hit.hasBlock) {
		PxActor* hitActor = hit.block.actor;
		for (size_t i = 0; i < mirrorActors.size(); ++i) {
			//if hitActor contained in list mirrorActors[i]
			if (std::find(mirrorActors[i].begin(), mirrorActors[i].end(), hitActor) != mirrorActors[i].end()) {
				if (hit.block.distance <= mirrorMaxDistances[i]) {
					return i;
				}
				else {
					return -1;
				}
			}
		}
	}
	return -1;
}