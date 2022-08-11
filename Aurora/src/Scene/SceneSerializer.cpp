#include "Aurorapch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

	// Provided on the yaml-cpp wiki
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& right)
		{
			Node node;
			node.push_back(right.x);
			node.push_back(right.y);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, glm::vec2& right)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			right.x = node[0].as<float>();
			right.y = node[1].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& right)
		{
			Node node;
			node.push_back(right.x);
			node.push_back(right.y);
			node.push_back(right.z);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, glm::vec3& right)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			right.x = node[0].as<float>();
			right.y = node[1].as<float>();
			right.z = node[2].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& right)
		{
			Node node;
			node.push_back(right.x);
			node.push_back(right.y);
			node.push_back(right.z);
			node.push_back(right.w);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, glm::vec4& right)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			right.x = node[0].as<float>();
			right.y = node[1].as<float>();
			right.z = node[2].as<float>();
			right.w = node[3].as<float>();

			return true;
		}
	};

}

namespace Aurora {

	// Provided on the yaml-cpp wiki
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << YAML::EndSeq;

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;

		return out;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << "129861987"; // TODO: Change when UUIDS are added

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // Tag Component

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // Tag Component
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // Transform Component

			auto& transform = entity.GetComponent<TransformComponent>();

			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			out << YAML::EndMap; // Transform Component
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // Camera Component

			auto& cameraComp = entity.GetComponent<CameraComponent>();

			out << YAML::Key << "Camera";
			out << YAML::BeginMap; // Camera!


			switch ((int)cameraComp.Camera.GetProjectionType())
			{
			    case 0: out << YAML::Key << "ProjectionType" << YAML::Value << 0; out << YAML::Comment("Perspective"); break;
			    case 1: out << YAML::Key << "ProjectionType" << YAML::Value << 1; out << YAML::Comment("Orthographic"); break;
			}

			out << YAML::Key << "PerspectiveFOV" << YAML::Value << cameraComp.Camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << cameraComp.Camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << cameraComp.Camera.GetPerspectiveFarClip();

			out << YAML::Key << "OrthographicSize" << YAML::Value << cameraComp.Camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << cameraComp.Camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << cameraComp.Camera.GetOrthographicFarClip();

			out << YAML::EndMap; // Camera!

			out << YAML::Key << "Primary" << YAML::Value << cameraComp.Primary;

			out << YAML::EndMap; // Camera Component
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // Sprite Renderer Component

			auto& color = entity.GetComponent<SpriteRendererComponent>().Color;

			out << YAML::Key << "Color" << YAML::Value << color;

			out << YAML::EndMap; // Sprite Renderer Component
		}

		out << YAML::EndMap; // Entity
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SceneSerializer::SerializeToText(const std::string& filepath)
	{
		AR_PROFILE_FUNCTION();

		YAML::Emitter outPut;

		outPut << YAML::BeginMap;

		outPut << YAML::Key << "Scene" << YAML::Value << "UnNamed"; // TODO: Add scene names
		outPut << YAML::Key << "Entities" << YAML::Value;

		outPut << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.raw() };
			if (!entity)
				return;

			SerializeEntity(outPut, entity);
		});
		outPut << YAML::EndSeq;

		outPut << YAML::EndMap;

		size_t lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash;
		std::string fileSysPath = filepath.substr(0, lastSlash);

		if (!std::filesystem::exists(fileSysPath)) // If filepath provided does not exist, the api creates it for you
			std::filesystem::create_directories(fileSysPath);

		std::ofstream fout(filepath);
		fout << outPut.c_str();
	}

	void SceneSerializer::SerializeToBinary(const std::string& filepath)
	{
		AR_CORE_ASSERT(false, "SceneSerializer", "Not Implemented!");
	}

	bool SceneSerializer::DeSerializeFromText(const std::string& filepath)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(std::filesystem::exists(filepath), "SceneSerializer", "Path does not exist");

		YAML::Node data;

		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			AR_CORE_ERROR_TAG("SceneSerializer", "Failed to load.aurora file '{0}'\n\t{1}", filepath, e.what());
		}

		if (!data["Scene"])
			return false; // If the file we are loading does not contain the Scene tag in the beginning we return since every serialized file should start with Scene

		std::string sceneName = data["Scene"].as<std::string>();
		AR_CORE_TRACE_TAG("SceneSerializer", "Deserializing scene '{0}'", sceneName);

		YAML::Node entities = data["Entities"]; // This is the entities node that exists under the scene
		if (entities)
		{
			for (auto entity : entities) // This loops and gets all the child nodes of the node "Entities"
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO: Comeback to this when UUIDs are implemented

				std::string entityName;
				YAML::Node tagComponent = entity["TagComponent"];
				if (tagComponent)
				{
					entityName = tagComponent["Tag"].as<std::string>();
				}

				AR_CORE_TRACE_TAG("SceneSerializer", "Deserialized entity with ID '{0}', name '{1}'", uuid, entityName);

				Entity deserializedEntity = m_Scene->CreateEntity(entityName.c_str());

				YAML::Node transform = entity["TransformComponent"];
				if (transform)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>(); // Since entities always have transform, that the way i made in the first place since it makes sense

					tc.Translation = transform["Translation"].as<glm::vec3>();
					tc.Rotation = transform["Rotation"].as<glm::vec3>();
					tc.Scale = transform["Scale"].as<glm::vec3>();
				}

				YAML::Node cameraComp = entity["CameraComponent"];
				if (cameraComp)
				{
					CameraComponent& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto& cameraProps = cameraComp["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)(cameraProps["ProjectionType"].as<int>()));

					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComp["Primary"].as<bool>();
				}

				YAML::Node spriteRendComp = entity["SpriteRendererComponent"];
				if (spriteRendComp)
				{
					glm::vec4& color = deserializedEntity.AddComponent<SpriteRendererComponent>().Color;

					color = spriteRendComp["Color"].as<glm::vec4>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeSerializeFromBinary(const std::string& filepath)
	{
		AR_CORE_ASSERT(false, "SceneSerializer", "Not Implemented!");

		return false;
	}

}