#include "Aurorapch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"
#include "AssetManager/AssetManager.h"
#include "Renderer/Renderer.h"

#include <yaml-cpp/yaml.h>
#include "Utils/YAMLSerializationHelpers.h"

namespace Aurora {

	namespace Utils {

		SceneCamera::ProjectionType ProjectionTypeFromString(std::string_view type)
		{
			if (type == "Perspective") return SceneCamera::ProjectionType::Perspective;
			if (type == "Orthographic") return SceneCamera::ProjectionType::Orthographic;

			AR_CORE_ASSERT(false, "Unknown type!");
			return (SceneCamera::ProjectionType)0;
		}

		std::string ProjectionTypeToString(SceneCamera::ProjectionType type)
		{
			switch (type)
			{
				case SceneCamera::ProjectionType::Perspective:  return "Perspective";
				case SceneCamera::ProjectionType::Orthographic: return "Orthographic";
			}

			AR_CORE_ASSERT(false, "Unknown type!");
			return "";
		}

	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			const std::string& tag = entity.GetComponent<TagComponent>().Tag;

			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // Tag Component

			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // Tag Component
		}

		if (entity.HasComponent<TransformComponent>())
		{
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();

			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // Transform Component

			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			out << YAML::EndMap; // Transform Component
		}

		if (entity.HasComponent<CameraComponent>())
		{
			const CameraComponent& cameraComp = entity.GetComponent<CameraComponent>();

			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // Camera Component

			out << YAML::Key << "Camera";
			out << YAML::BeginMap; // Camera!

			out << YAML::Key << "ProjectionType" << YAML::Value << Utils::ProjectionTypeToString(cameraComp.Camera.GetProjectionType());

			// Degrees
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << cameraComp.Camera.GetDegPerspectiveVerticalFOV();
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
			const SpriteRendererComponent& src = entity.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // Sprite Renderer Component

			out << YAML::Key << "Color" << YAML::Value << src.Color;
			out << YAML::Key << "TilingFactor" << YAML::Value << src.TilingFactor;

			out << YAML::EndMap; // Sprite Renderer Component
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			const CircleRendererComponent& crc = entity.GetComponent<CircleRendererComponent>();

			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // Circle Renderer Component

			out << YAML::Key << "Color" << YAML::Value << crc.Color;
			out << YAML::Key << "Thickness" << YAML::Value << crc.Thickness;

			out << YAML::EndMap; // Circle Renderer Component
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			const DirectionalLightComponent& dlc = entity.GetComponent<DirectionalLightComponent>();

			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap; // Directional Light Component

			out << YAML::Key << "Radiance" << YAML::Value << dlc.Radiance;
			out << YAML::Key << "Intensity" << YAML::Value << dlc.Intensity;

			out << YAML::EndMap; // Directional Light Component
		}

		if (entity.HasComponent<SkyLightComponent>())
		{
			const SkyLightComponent& slc = entity.GetComponent<SkyLightComponent>();

			out << YAML::Key << "SkyLightComponent";
			out << YAML::BeginMap; // Sky Light Component

			// TODO: Should store the UUID instead of the filepath once there is an asset manager
			if (slc.SceneEnvironment)
				out << YAML::Key << "EnvironmentMap" << YAML::Value << slc.SceneEnvironment->RadianceMap->GetAssetPath().string();
			else
				out << YAML::Key << "EnvironmentMap" << YAML::Value << "";

			out << YAML::Key << "LOD" << YAML::Value << slc.Level;
			out << YAML::Key << "Intensity" << YAML::Value << slc.Intensity;
			out << YAML::Key << "DynamicSky" << YAML::Value << slc.DynamicSky;
			if (slc.DynamicSky)
			{
				out << YAML::Key << "Turbidity" << YAML::Value << slc.TurbidityAzimuthInclination.x;
				out << YAML::Key << "Azimuth" << YAML::Value << slc.TurbidityAzimuthInclination.y;
				out << YAML::Key << "Inclination" << YAML::Value << slc.TurbidityAzimuthInclination.z;
			}

			out << YAML::EndMap; // Sky Light Component
		}

		if (entity.HasComponent<TextComponent>())
		{
			const TextComponent& tc = entity.GetComponent<TextComponent>();

			out << YAML::Key << "TextComponent";
			out << YAML::BeginMap; // Text Component

			out << YAML::Key << "TextString" << YAML::Value << tc.TextString;
			//out << YAML::Key << "FontHandle" << YAML::Value << tc.FontHandle; // TODO: When Asset Manager exists
			out << YAML::Key << "FontHandle" << YAML::Value << 0;
			out << YAML::Key << "Color" << YAML::Value << tc.Color;
			out << YAML::Key << "LineSpacing" << YAML::Value << tc.LineSpacing;
			out << YAML::Key << "Kerning" << YAML::Value << tc.Kerning;
			out << YAML::Key << "MaxWidth" << YAML::Value << tc.MaxWidth;

			out << YAML::EndMap; // Text Component
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

		outPut << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();
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
		AR_CORE_ASSERT(false, "Not Implemented");
	}

	bool SceneSerializer::DeSerializeFromText(const std::string& filepath)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(std::filesystem::exists(filepath), "Path does not exist");

		YAML::Node data;

		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			AR_CORE_ERROR_TAG("SceneSerializer", "Failed to load scene file '{0}'\n\t{1}", filepath, e.what());
		}

		// If the file we are loading does not contain the Scene tag in the beginning we return since every serialized file should start with Scene
		if (!data["Scene"])
			return false;

		std::string& sceneName = m_Scene->GetName();
		sceneName = data["Scene"].as<std::string>();
		AR_CORE_TRACE_TAG("SceneSerializer", "Deserializing scene '{0}'", sceneName); // TODO: Display the UUID of the scene when that is a thing

		YAML::Node entities = data["Entities"]; // This is the entities node that exists under the scene
		if (entities)
		{
			for (auto entity : entities) // This loops and gets all the child nodes of the node "Entities"
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string entityName;
				YAML::Node tagComponent = entity["TagComponent"];
				if (tagComponent)
				{
					entityName = tagComponent["Tag"].as<std::string>();
				}

				AR_CORE_TRACE_TAG("SceneSerializer", "Deserialized entity with ID: {0:#04x}, Name: {1}", uuid, entityName);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, entityName);

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
					cc.Camera.SetProjectionType(Utils::ProjectionTypeFromString(cameraProps["ProjectionType"].as<std::string>()));

					cc.Camera.SetDegPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
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
					SpriteRendererComponent& src = deserializedEntity.AddComponent<SpriteRendererComponent>();

					src.Color = spriteRendComp["Color"].as<glm::vec4>();
					src.TilingFactor = spriteRendComp["TilingFactor"].as<float>();
				}

				YAML::Node circleRendComp = entity["CircleRendererComponent"];
				if (circleRendComp)
				{
					CircleRendererComponent& crc = deserializedEntity.AddComponent<CircleRendererComponent>();

					crc.Color = circleRendComp["Color"].as<glm::vec4>();
					crc.Thickness = circleRendComp["Thickness"].as<float>();
				}

				YAML::Node dirLightComponent = entity["DirectionalLightComponent"];
				if (dirLightComponent)
				{
					DirectionalLightComponent& dlc = deserializedEntity.AddComponent<DirectionalLightComponent>();

					dlc.Radiance = dirLightComponent["Radiance"].as<glm::vec3>();
					dlc.Intensity = dirLightComponent["Intensity"].as<float>();
				}

				YAML::Node skyLightComponent = entity["SkyLightComponent"];
				if (skyLightComponent)
				{
					SkyLightComponent& slc = deserializedEntity.AddComponent<SkyLightComponent>();

					// TODO: Should change from storing the path to actually just getting the uuid
					const std::string envMapPath = skyLightComponent["EnvironmentMap"].as<std::string>();
					if (!envMapPath.empty())
					{
						const auto& [radianceMap, irradianceMap] = Renderer::CreateEnvironmentMap(envMapPath);
						slc.SceneEnvironment = Environment::Create(radianceMap, irradianceMap);
					}
					else
					{
						slc.SceneEnvironment = nullptr;
					}

					slc.Level = skyLightComponent["LOD"].as<float>();
					slc.Intensity = skyLightComponent["Intensity"].as<float>();

					slc.DynamicSky = skyLightComponent["DynamicSky"].as<bool>();
					if (slc.DynamicSky)
					{
						float turbidity = skyLightComponent["Turbidity"].as<float>();
						float azimuth = skyLightComponent["Azimuth"].as<float>();
						float inclination = skyLightComponent["Inclination"].as<float>();

						slc.TurbidityAzimuthInclination = glm::vec3{ turbidity, azimuth, inclination };
					}
				}

				YAML::Node textComponent = entities["TextComponent"];
				if (textComponent)
				{
					TextComponent& tc = deserializedEntity.AddComponent<TextComponent>();

					tc.TextString = textComponent["TextString"].as<std::string>();
					tc.TextHash = std::hash<std::string>()(tc.TextString);

					// TODO:
					//AssetHandle fontHandle = textComponent["FontHandle"].as<uint64_t>();
					//if (AssetManager::IsAssetHandleValid(fontHandle))
					//	tc.FontHandle = fontHandle;
					//else
					//	tc.FontHandle = Font::GetDefaultFont()->Handle;
					tc.FontHandle = 0;
					tc.Color = textComponent["Color"].as<glm::vec4>();
					tc.LineSpacing = textComponent["LineSpacing"].as<float>();
					tc.Kerning = textComponent["Kerning"].as<float>();
					tc.MaxWidth = textComponent["MaxWidth"].as<float>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeSerializeFromBinary(const std::string& filepath)
	{
		AR_CORE_ASSERT(false, "Not Implemented");

		return false;
	}

}