#include "Ragdoll.h"
#include <math.h>


Ragdoll::Ragdoll()
{
}

void Ragdoll::RemovePhysicsObjects()
{
	for (RigidComponent& rigid : m_rigidComponents)
		rigid.pxRigidBody->release();
	
	for (JointComponent& joint : m_jointComponents)
		joint.pxD6->release();
}


void Ragdoll::BuildFromJsonFile(std::string filename, Transform spawnLocation, void* parent, PhysicsObjectType type)
{
	// this is the object that owns the ragdoll
	//m_parent = parent;

	RemovePhysicsObjects();

	// Load file
	std::string fileName = "res/" + filename;
	FILE* pFile = fopen(fileName.c_str(), "rb");
	char buffer[65536];
	rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
	rapidjson::Document document;
	document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);

	// Check for errors
	if (document.HasParseError())
		std::cout << "Error  : " << document.GetParseError() << '\n' << "Offset : " << document.GetErrorOffset() << '\n';

	// Loop over entities
	for (auto& entity : document["entities"].GetObject())
	{
		auto const components = entity.value["components"].GetObject();
		if (components.HasMember("RigidComponent"))
		{
			RigidComponent rigidComponent;
			rigidComponent.ID = entity.value["id"].GetInt();
			rigidComponent.name = components["NameComponent"].GetObject()["members"].GetObject()["value"].GetString();
			rigidComponent.restMatrix = Util::Mat4FromJSONArray(components["RestComponent"].GetObject()["members"].GetObject()["matrix"].GetObject()["values"].GetArray());
			rigidComponent.scaleAbsoluteVector = Util::Vec3FromJSONArray(components["ScaleComponent"].GetObject()["members"].GetObject()["absolute"].GetObject()["values"].GetArray());
			rigidComponent.capsuleRadius = components["GeometryDescriptionComponent"].GetObject()["members"].GetObject()["radius"].GetFloat();
			rigidComponent.capsuleLength = components["GeometryDescriptionComponent"].GetObject()["members"].GetObject()["length"].GetFloat();
			rigidComponent.shapeType = components["GeometryDescriptionComponent"].GetObject()["members"].GetObject()["type"].GetString();
			rigidComponent.boxExtents = Util::Vec3FromJSONArray(components["GeometryDescriptionComponent"].GetObject()["members"].GetObject()["extents"].GetObject()["values"].GetArray());
			rigidComponent.offset = Util::Vec3FromJSONArray(components["GeometryDescriptionComponent"].GetObject()["members"].GetObject()["offset"].GetObject()["values"].GetArray());
			rigidComponent.rotation = Util::PxQuatFromJSONArray(components["GeometryDescriptionComponent"].GetObject()["members"].GetObject()["rotation"].GetObject()["values"].GetArray());
			rigidComponent.mass = components["RigidComponent"].GetObject()["members"].GetObject()["mass"].GetFloat();
			rigidComponent.friction = components["RigidComponent"].GetObject()["members"].GetObject()["friction"].GetFloat();
			rigidComponent.restitution = components["RigidComponent"].GetObject()["members"].GetObject()["restitution"].GetFloat();
			rigidComponent.linearDamping = components["RigidComponent"].GetObject()["members"].GetObject()["linearDamping"].GetFloat();
			rigidComponent.angularDamping = components["RigidComponent"].GetObject()["members"].GetObject()["angularDamping"].GetFloat();
			rigidComponent.sleepThreshold = components["RigidComponent"].GetObject()["members"].GetObject()["sleepThreshold"].GetFloat();
			rigidComponent.correspondingJointName = FindParentJointName(components["NameComponent"].GetObject()["members"].GetObject()["path"].GetString());

			rigidComponent.angularMass = Util::PxVec3FromJSONArray(components["RigidComponent"].GetObject()["members"].GetObject()["angularMass"].GetObject()["values"].GetArray());

			if (rigidComponent.name != "rSceneShape" && rigidComponent.correspondingJointName != "rScene")
				m_rigidComponents.push_back(rigidComponent);
		}

		if (components.HasMember("JointComponent"))
		{
			//std::cout << "ID: " << entity.value["id"].GetInt() << "\n";

			JointComponent jointComponent;
			jointComponent.name = components["NameComponent"].GetObject()["members"].GetObject()["value"].GetString();
			jointComponent.parentID = components["JointComponent"].GetObject()["members"].GetObject()["parent"].GetObject()["value"].GetInt();
			jointComponent.childID = components["JointComponent"].GetObject()["members"].GetObject()["child"].GetObject()["value"].GetInt();
			jointComponent.parentFrame = Util::PxMat4FromJSONArray(components["JointComponent"].GetObject()["members"].GetObject()["parentFrame"].GetObject()["values"].GetArray());
			jointComponent.childFrame = Util::PxMat4FromJSONArray(components["JointComponent"].GetObject()["members"].GetObject()["childFrame"].GetObject()["values"].GetArray());
			jointComponent.drive_angularDamping = components["DriveComponent"].GetObject()["members"].GetObject()["angularDamping"].GetFloat();
			jointComponent.drive_angularStiffness = components["DriveComponent"].GetObject()["members"].GetObject()["angularStiffness"].GetFloat();
			jointComponent.drive_linearDampening = components["DriveComponent"].GetObject()["members"].GetObject()["linearDamping"].GetFloat();
			jointComponent.drive_linearStiffness = components["DriveComponent"].GetObject()["members"].GetObject()["linearStiffness"].GetFloat();
			jointComponent.drive_enabled = components["DriveComponent"].GetObject()["members"].GetObject()["enabled"].GetBool(); 
			jointComponent.target = Util::Mat4FromJSONArray(components["DriveComponent"].GetObject()["members"].GetObject()["target"].GetObject()["values"].GetArray());
			jointComponent.limit_angularDampening = components["LimitComponent"].GetObject()["members"].GetObject()["angularDamping"].GetFloat();
			jointComponent.limit_angularStiffness = components["LimitComponent"].GetObject()["members"].GetObject()["angularStiffness"].GetFloat();
			jointComponent.limit_linearDampening = components["LimitComponent"].GetObject()["members"].GetObject()["linearDamping"].GetFloat();
			jointComponent.limit_linearStiffness = components["LimitComponent"].GetObject()["members"].GetObject()["linearStiffness"].GetFloat();
			jointComponent.twist = components["LimitComponent"].GetObject()["members"].GetObject()["twist"].GetFloat();
			jointComponent.swing1 = components["LimitComponent"].GetObject()["members"].GetObject()["swing1"].GetFloat();
			jointComponent.swing2 = components["LimitComponent"].GetObject()["members"].GetObject()["swing2"].GetFloat();
			jointComponent.limit.x = components["LimitComponent"].GetObject()["members"].GetObject()["x"].GetFloat();
			jointComponent.limit.y = components["LimitComponent"].GetObject()["members"].GetObject()["y"].GetFloat();
			jointComponent.limit.z = components["LimitComponent"].GetObject()["members"].GetObject()["z"].GetFloat();
			jointComponent.joint_enabled = components["LimitComponent"].GetObject()["members"].GetObject()["enabled"].GetBool();
			m_jointComponents.push_back(jointComponent);

		}
	}

	//std::cout << "m_rigidComponents sizzze:  " << m_rigidComponents.size() << "\n";

	PxScene* scene;
	PxPhysics& physX = PxGetPhysics();
	PxGetPhysics().getScenes(&scene, 1);


	for (RigidComponent& rigid : m_rigidComponents)
	{
		


			PxShape* shape;
			auto gMaterial = physX.createMaterial(0.5f, 0.5f, 0.6f);

			// Skip the scene rigid (it's outputted in the JSON export)
			if (rigid.name == "rSceneShape")
				continue;
			// Apply scale
			if (rigid.shapeType == "Capsule") {
				rigid.capsuleRadius *= rigid.scaleAbsoluteVector.x;
				rigid.capsuleLength *= rigid.scaleAbsoluteVector.y;
			}
			else if (rigid.shapeType == "Box") {
				rigid.boxExtents.x *= rigid.scaleAbsoluteVector.x;
				rigid.boxExtents.y *= rigid.scaleAbsoluteVector.y;
				rigid.boxExtents.z *= rigid.scaleAbsoluteVector.z;
			}

			// Create PhysX shapes
			if (rigid.shapeType == "Capsule") {
				float halfExtent = rigid.capsuleLength * 0.5;
				float radius = rigid.capsuleRadius;
				shape = physX.createShape(PxCapsuleGeometry(radius, halfExtent), *gMaterial);
			}
			else if (rigid.shapeType == "Box") {
				float halfExtent = rigid.capsuleLength;
				float radius = rigid.capsuleRadius;
				shape = physX.createShape(PxBoxGeometry(rigid.boxExtents.x * 0.5, rigid.boxExtents.y * 0.5, rigid.boxExtents.z * 0.5), *gMaterial);
			}

			PxMat44 restMatrix = Util::GlmMat4ToPxMat44(rigid.restMatrix);

			PxTransform offsetTranslation = PxTransform(PxVec3(rigid.offset.x, rigid.offset.y, rigid.offset.z));
			PxTransform offsetRotation = PxTransform(rigid.rotation);

			shape->setLocalPose(offsetTranslation.transform(offsetRotation));
		
			PxMat44 spawnMatrix = Util::GlmMat4ToPxMat44(spawnLocation.to_mat4());


			rigid.pxRigidBody = physX.createRigidDynamic(PxTransform(spawnMatrix * restMatrix));
			rigid.pxRigidBody->attachShape(*shape);
			rigid.pxRigidBody->setSolverIterationCounts(8, 1);
			rigid.pxRigidBody->setName("RAGDOLL");	

			if (rigid.correspondingJointName == "mixamorig:Head" ||
				rigid.correspondingJointName == "mixamorig:Neck")
			{
				rigid.pxRigidBody->setName("RAGDOLL_HEAD");
			}


			rigid.pxRigidBody->userData = new EntityData(type, parent);
			scene->addActor(*rigid.pxRigidBody);
			PxRigidBodyExt::setMassAndUpdateInertia(*rigid.pxRigidBody, rigid.mass);

		//	std::cout << rigid.correspondingJointName << '\n';

			PhysX::EnableRayCastingForShape(shape);


			shape->release();

			//PxFilterData data;
			//data.word0 = 1452323;
			//shape->setQueryFilterData(data);

				  

			// PUT TO SLEEP AT START UP!
			rigid.pxRigidBody->putToSleep();
	}


	scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);



		
	for (JointComponent& joint : m_jointComponents)
	{


		// Get pointers to the parent and child rigid bodies
		RigidComponent* parentRigid;
		RigidComponent* childRigid;

		for (RigidComponent& rigid : m_rigidComponents)
		{
			if (rigid.ID == joint.parentID)
				parentRigid = &rigid;
			if (rigid.ID == joint.childID)
				childRigid = &rigid;
		}


		PxTransform parentFrame = PxTransform(joint.parentFrame);
		PxTransform childFrame = PxTransform(joint.childFrame);
		joint.pxD6 = PxD6JointCreate(physX, parentRigid->pxRigidBody, parentFrame, childRigid->pxRigidBody, childFrame);

		joint.pxD6->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);
		
		joint.pxD6->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
	
			const PxSpring linearSpring{
			joint.limit_linearStiffness,
			joint.limit_linearDampening
		};


		const PxJointLinearLimitPair limitX{
			-joint.limit.x,
			joint.limit.x,
			linearSpring
		};
		joint.pxD6->setLinearLimit(PxD6Axis::eX, limitX);

		const PxJointLinearLimitPair limitY{
			-joint.limit.y,
			joint.limit.y,
			linearSpring
		};
		joint.pxD6->setLinearLimit(PxD6Axis::eY, limitY);

		const PxJointLinearLimitPair limitZ{
			-joint.limit.z,
			joint.limit.z,
			linearSpring
		};
		joint.pxD6->setLinearLimit(PxD6Axis::eZ, limitZ);


		const PxSpring angularSpring{
			joint.drive_angularStiffness,
			joint.drive_angularDamping
		};
		const PxJointAngularLimitPair twistLimit{
			-joint.twist,
			 joint.twist,
			 angularSpring
		};
		joint.pxD6->setTwistLimit(twistLimit);

		const PxJointLimitCone swingLimit{
			joint.swing1,
			joint.swing2,
			angularSpring
		};
		joint.pxD6->setSwingLimit(swingLimit);
				
		if (joint.limit.x > 0) joint.pxD6->setMotion(PxD6Axis::eX, PxD6Motion::eLIMITED);
		if (joint.limit.y > 0) joint.pxD6->setMotion(PxD6Axis::eY, PxD6Motion::eLIMITED);
		if (joint.limit.z > 0) joint.pxD6->setMotion(PxD6Axis::eZ, PxD6Motion::eLIMITED);

		if (joint.limit.x < 0) joint.pxD6->setMotion(PxD6Axis::eX, PxD6Motion::eLOCKED);
		if (joint.limit.y < 0) joint.pxD6->setMotion(PxD6Axis::eY, PxD6Motion::eLOCKED);
		if (joint.limit.z < 0) joint.pxD6->setMotion(PxD6Axis::eZ, PxD6Motion::eLOCKED);

		if (joint.twist > 0) joint.pxD6->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
		if (joint.swing1 > 0) joint.pxD6->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		if (joint.swing2 > 0) joint.pxD6->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);

		if (joint.twist < 0) joint.pxD6->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLOCKED);
		if (joint.swing1 < 0) joint.pxD6->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLOCKED);
		if (joint.swing2 < 0) joint.pxD6->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);


		if (joint.drive_enabled) {
			PxD6JointDrive linearDrive{
			joint.limit_linearStiffness,
			joint.limit_linearDampening,
			FLT_MAX,   // Maximum force; ignored
			true,  // ACCELERATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! read from the json file some day when you aren't lazy
			};

			joint.pxD6->setDrive(PxD6Drive::eX, linearDrive);
			joint.pxD6->setDrive(PxD6Drive::eY, linearDrive);
			joint.pxD6->setDrive(PxD6Drive::eZ, linearDrive);

			PxD6JointDrive angularDrive{
				joint.drive_angularStiffness,
				joint.drive_angularDamping,

				// Internal defaults
				FLT_MAX,
				true,  // ACCELERATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! read from the json file some day when you aren't lazy
			};

			joint.pxD6->setDrive(PxD6Drive::eTWIST, angularDrive);
			joint.pxD6->setDrive(PxD6Drive::eSWING, angularDrive);
			joint.pxD6->setDrive(PxD6Drive::eSLERP, angularDrive);

			// Update target
			//
			// NOTE: Allow for changes to be made to both parent
			// and child frames, without affecting the drive target
			//
			// TODO: Unravel this. We currently can't edit the child anchorpoint
			/*MMatrix mat = joint.childFrame
				* drive.target
				* joint.parentFrame.inverse();
			// Take negative scale into account
			const auto parentScale = g.registry.get<ScaleComponent>(joint.parent);
			if (parentScale.value.x < 0 || parentScale.value.y < 0 || parentScale.value.z < 0) {
				mat = parentScale.matrix * mat;

				const MTransformationMatrix matTm{ mat };
				const MQuaternion quat = matTm.rotation();
				MTransformationMatrix finalTm{};
				finalTm.setTranslation(matTm.getTranslation(MSpace::kTransform), MSpace::kTransform);
				finalTm.setRotationQuaternion(quat.x, quat.y, quat.z, quat.w);

				mat = finalTm.asMatrix();
			}

			const PxTransform pxtransform{ toPxMat44(mat) };
			pxjoint->setDrivePosition(pxtransform);*/
		}

		else {
			static const PxD6JointDrive defaultDrive{ 0.0f, 0.0f, 0.0f, false };
			joint.pxD6->setDrive(PxD6Drive::eX, defaultDrive);
			joint.pxD6->setDrive(PxD6Drive::eY, defaultDrive);
			joint.pxD6->setDrive(PxD6Drive::eZ, defaultDrive);
			joint.pxD6->setDrive(PxD6Drive::eTWIST, defaultDrive);
			joint.pxD6->setDrive(PxD6Drive::eSWING, defaultDrive);
			joint.pxD6->setDrive(PxD6Drive::eSLERP, defaultDrive);
		}
	}
}




std::string Ragdoll::FindParentJointName(std::string query)
{
	return query.substr(query.rfind("|") + 1); 
	
	/*size_t occuranceCount = std::count(query.begin(), query.end(), '|'); // #include <algorithm>
	if (occuranceCount < 2)
		return "undefined";

	std::size_t lastOccurance = query.rfind("|");
	std::size_t secondLastOccurance = query.substr(0, lastOccurance).rfind("|") + 1;
	return query.substr(secondLastOccurance, lastOccurance - secondLastOccurance);*/
}

RigidComponent* Ragdoll::GetRigidByName(std::string name)
{
	//std::cout << "m_rigidComponents size: " << m_rigidComponents.size() << "\n";

	for (RigidComponent& rigidComponent : m_rigidComponents) {

		//std::cout << "rigidComponent: " << rigidComponent.correspondingJointName << "\n";

		if (rigidComponent.correspondingJointName == name) {
			return &rigidComponent;
		}
	}
	return nullptr;
}


