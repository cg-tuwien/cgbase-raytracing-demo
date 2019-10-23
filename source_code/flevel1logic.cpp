#include "includes.h"
#include "flevel1logic.h"

flevel1logic::flevel1logic(fscene* scene) : flevellogic(scene) {
	initialCameraPos = scene->get_camera().translation();
	initialCameraRot = scene->get_camera().rotation();
	physics = std::make_unique<fphysicscontroller>(scene);
	player = std::make_unique<fplayercontrol>(physics.get(), scene, false, 1.5, (PxUserControllerHitReport*)this);

	/*scene->materials[0].diffuseColor *= 2;
	scene->materials[2].ambientColor = glm::vec3(0.4f);
	scene->materials[3].ambientColor = glm::vec3(0.4f);*/

	std::vector<std::string> walls = {
		//"Wall1", "Wall2", "Wall3", "Wall4", "Wall6", "Wall7", "Wall8"
		"Plane.010", "Plane.002", "Plane.006", "Plane.005", "Plane.008", "Plane.009", "Plane.007"
	};

	std::vector<std::string> floors = {
		//"Floor1", "Floor2", "Floor3", "Floor4", "Floor5", "Floor6", "Floor7", "Floor8", "Floor9", "Floor10"
		"Cube.001", "Cube.000", "Cube.003", "Cylinder.002", "Cube.005", "Cube.006", "Cube.011", "Cube.012", "Cube.013", "Cube.002"
	};
	int i = 0;
	for (const std::string& name : walls) {
		auto searchres = scene->get_model_by_name(name);
		if (!searchres.has_value()) throw new std::runtime_error("Did not found " + name);
		fmodel* instance = searchres.value();
		physics->create_rigid_static_for_scaled_plane(instance);
	}
	for (const std::string& name : floors) {
		auto searchres = scene->get_model_by_name(name);
		if (!searchres.has_value()) throw new std::runtime_error("Did not found " + name);
		fmodel* instance = searchres.value();
		auto moving = i >= 6 && i <= 8;
		auto actor = physics->create_rigid_static_for_scaled_unit_box(instance, moving);
		if (moving) {
			movingFloors[i - 6] = actor;
		}
		++i;
	}

	auto finalRegionRes = scene->get_model_by_name("Cube.015");
	if (!finalRegionRes.has_value()) throw new std::runtime_error("Did not found FinalRegion");
	finalRegionActor = physics->create_rigid_static_for_scaled_unit_box(finalRegionRes.value(), true);
	player->set_final_region(finalRegionActor);
	////finalRegionInstance->shaderOffset = 2;
	////finalRegionInstance->opaque = false;

	//sphereInstance = &scene->findInstanceByName("Sphere_0", found);
	//if (!found) throw new std::runtime_error("Did not find Sphere!");
	//auto mirrorBorderInstance = &scene->findInstanceByName("MirrorBorder1_0", found);
	//if (!found) throw new std::runtime_error("Did not find MirrorBorder1");
	//auto mirrorPlaneInstance = &scene->findInstanceByName("MirrorPlane1_0", found);
	//if (!found) throw new std::runtime_error("Did not find MirrorPlane1");

	//mirrorBorderActor = physics->createRigidStaticForScaledUnitBox(mirrorBorderInstance, true);
	//mirrorPlaneActor = physics->createRigidStaticForScaledPlane(mirrorPlaneInstance, true);
	//player->addMirror({ mirrorBorderActor, mirrorPlaneActor });

	////Create Sphere
	//sphereInstance->flags = 1;

	//interpolator.addSample(0, glm::vec3(47, 0, 0.3));
	//interpolator.addSample(1, glm::vec3(60, 1, 1));
}

levelstatus flevel1logic::update(const float& deltaT, const double& focusHitValue)
{
	/*if (score > 2.0f) {
		return WON;
	}
	if (std::fabs(focusHitValue) < 0.0001) {
		score = glm::max(score - 0.08 * deltaT, 0.0);
	}
	else {
		score = glm::min(score + deltaT * 0.1, 1.0);
	}

	scene->backgroundColor = interpolator.interpolate(score);
	if ((score >= 0.99 && player->onFinalRegion()) || glfwGetKey(app->window, GLFW_KEY_F10) == GLFW_PRESS) {
		scene->materials[sphereInstance->materialIndex].diffuseColor = glm::vec3(1.0f);
		scene->materialUpdated = glfwGetTime();
		scene->backgroundColor = glm::vec3(1, 1, 0.5);
		score = 100.0f;
		return WON;
	}
	if (player->fellDown()) {
		return LOST;
	}*/
	return RUNNING;
}

void flevel1logic::fixed_update(const float& stepSize)
{
	accTime += stepSize;
	float xA = -(22 + 1.5 * cos(2 * accTime));
	float xB = -(22 - 1.5 * cos(2 * accTime));
	PxTransform oldpose[4];
	PxTransform newpose[4];
	oldpose[0] = movingFloors[0]->getGlobalPose();
	newpose[0] = PxTransform(PxVec3(xA, oldpose[0].p.y, oldpose[0].p.z), oldpose[0].q);
	movingFloors[0]->setGlobalPose(newpose[0]);
	oldpose[1] = movingFloors[1]->getGlobalPose();
	newpose[1] = PxTransform(PxVec3(xB, oldpose[1].p.y, oldpose[1].p.z), oldpose[1].q);
	movingFloors[1]->setGlobalPose(newpose[1]);
	oldpose[2] = movingFloors[2]->getGlobalPose();
	newpose[2] = PxTransform(PxVec3(xA, oldpose[2].p.y, oldpose[2].p.z), oldpose[2].q);
	movingFloors[2]->setGlobalPose(newpose[2]);
	oldpose[3] = finalRegionActor->getGlobalPose();
	newpose[3] = PxTransform(PxVec3(xB, oldpose[3].p.y, oldpose[3].p.z), oldpose[3].q);
	finalRegionActor->setGlobalPose(newpose[3]);
	player->pre_px_update(stepSize);
	physics->update(stepSize);
	player->post_px_update(stepSize);

	for (int i = 0; i < 4; ++i) {
		if (onPlatform[i]) {
			PxVec3 diffvec = newpose[i].p - oldpose[i].p;
			glm::vec3 distance = glm::make_vec3(&diffvec.x);
			player->beam_along(distance);
			onPlatform[i] = false;
		}
	}
}

void flevel1logic::reset()
{
	accTime = 0;
	scene->get_camera().set_rotation(initialCameraRot);
	scene->get_camera().set_translation(initialCameraPos);
	player->update_position();
	player->reset_mirrors();
	//score = 0;
}

void flevel1logic::onShapeHit(const PxControllerShapeHit& hit)
{
	for (int i = 0; i < 3; ++i) {
		if (hit.actor == movingFloors[i]) {
			onPlatform[i] = true;
		}
	}
	if (hit.actor == finalRegionActor) {
		onPlatform[3] = true;
	}
}
