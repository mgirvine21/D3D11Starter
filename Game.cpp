#include "Graphics.h"
#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include <math.h>
#include "BufferStructs.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include "WICTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	ImGui::StyleColorsDark();

	//create camera
	float aspectRatio = Window::AspectRatio();
	XMFLOAT3 persStartPos = { 6.0f, 1.0f, -12.0f };
	XMFLOAT3 orthoStartPos = { 0.0f, 0.0f, -2.0f };
	cameras.push_back(std::make_shared<Camera>(aspectRatio, persStartPos, XM_PIDIV4, true)); // perspective cam 1
	cameras.push_back(std::make_shared<Camera>(aspectRatio, orthoStartPos, XM_PIDIV4 + 10, true)); // perspective cam 2
	activeCameraIndex = 0;
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//loading textures
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler; 
	//create a sampler state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&sampDesc, sampler.GetAddressOf());

	//actually load a texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/rock.png").c_str(), 0, rockSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/wood.png").c_str(), 0, woodSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/cobble.png").c_str(), 0, cobbleSRV.GetAddressOf());

	//creating shaders
	std::shared_ptr<SimpleVertexShader> vertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> uvShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> normalShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> customShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> multiplyShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"MultiplyPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> lightingShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelLightingShader.cso").c_str());

	//loading models
	std::shared_ptr<Mesh> sphereMesh0 = std::make_shared<Mesh>("sphere0", FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh1 = std::make_shared<Mesh>("sphere1", FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh2 = std::make_shared<Mesh>("sphere2", FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh3 = std::make_shared<Mesh>("sphere3", FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh4 = std::make_shared<Mesh>("sphere4", FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>("cube", FixPath(L"../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>("helix", FixPath(L"../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>("torus", FixPath(L"../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>("cylinder", FixPath(L"../../Assets/Models/cylinder.obj").c_str());
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>("quad", FixPath(L"../../Assets/Models/quad.obj").c_str());
	std::shared_ptr<Mesh> quad_double_sidedMesh = std::make_shared<Mesh>("quad_double_sided", FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str());

	//updating mesh vector
	meshes.insert(meshes.end(), { sphereMesh0, sphereMesh1, sphereMesh2, sphereMesh3, sphereMesh4, cubeMesh, helixMesh, torusMesh, cylinderMesh, quadMesh, quad_double_sidedMesh });

	//creating materials
	std::shared_ptr<Material> mat1 = std::make_shared<Material>(pixelShader, vertexShader, XMFLOAT3(.75f, 0, 0.95f), 0.0f, "Rocky Purple");
	mat1->AddSampler("BasicSampler", sampler);
	mat1->AddTextureSRV("SurfaceTexture", rockSRV);

	std::shared_ptr<Material> matUV = std::make_shared<Material>(uvShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "UV Preview", XMFLOAT2(1, 1));
	std::shared_ptr<Material> matNorm = std::make_shared<Material>(normalShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Normal Preview", XMFLOAT2(1, 1));
	std::shared_ptr<Material> matCustom = std::make_shared<Material>(customShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Custom Colorshift", XMFLOAT2(1, 1));

	//adding texture to materials
		std::shared_ptr<Material> matRock = std::make_shared<Material>(pixelShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Rock", XMFLOAT2(1, 1));
	matRock->AddSampler("BasicSampler", sampler);
	matRock->AddTextureSRV("SurfaceTexture", rockSRV);

	std::shared_ptr<Material> matWood = std::make_shared<Material>(pixelShader, vertexShader, XMFLOAT3(1,1,1), 0.0f, "Wood", XMFLOAT2(2, 2));
	matWood->AddSampler("BasicSampler", sampler);
	matWood->AddTextureSRV("SurfaceTexture", woodSRV);
	
	//mat with two textures using the multiply pixle shader
	std::shared_ptr<Material> matCobbleStone = std::make_shared<Material>(multiplyShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Cobble", XMFLOAT2(1, 1));
	matCobbleStone->AddSampler("BasicSampler", sampler);
	matCobbleStone->AddTextureSRV("SurfaceTextureStone", rockSRV);
	matCobbleStone->AddTextureSRV("SurfaceTextureCobble", cobbleSRV);
	
	//mat that uses lighting
	std::shared_ptr<Material> matLighting = std::make_shared<Material>(lightingShader, vertexShader, XMFLOAT3(.75f, 0, 0.95f), 0.0f, "Lighting");
	matLighting->AddSampler("BasicSampler", sampler);
	matLighting->AddTextureSRV("SurfaceTexture", rockSRV);

	//updating mats vector
	mats.insert(mats.end(), { mat1, matUV, matNorm, matCustom, matRock, matWood, matCobbleStone, matLighting });

	//updating entities vector
	entities.push_back(std::make_shared<GameEntity>(sphereMesh0, matLighting));
	entities.push_back(std::make_shared<GameEntity>(cubeMesh, matLighting));
	entities.push_back(std::make_shared<GameEntity>(helixMesh, matLighting));
	entities.push_back(std::make_shared<GameEntity>(torusMesh, matLighting));
	entities.push_back(std::make_shared<GameEntity>(cylinderMesh, matLighting));
	entities.push_back(std::make_shared<GameEntity>(quadMesh, matLighting));
	entities.push_back(std::make_shared<GameEntity>(quad_double_sidedMesh, matLighting));
	entities.push_back(std::make_shared<GameEntity>(sphereMesh1, matLighting));
	entities.push_back(std::make_shared<GameEntity>(sphereMesh2, matLighting));
	entities.push_back(std::make_shared<GameEntity>(sphereMesh3, matLighting));
	entities.push_back(std::make_shared<GameEntity>(sphereMesh4, matLighting));

	//place entities in scene
	entities[0]->GetTransform()->MoveAbsolute(-3, 0, 5);
	entities[1]->GetTransform()->MoveAbsolute(0, 0, 5);
	entities[2]->GetTransform()->MoveAbsolute(3, 0, 5);
	entities[3]->GetTransform()->MoveAbsolute(6, 0, 5);
	entities[4]->GetTransform()->MoveAbsolute(9, 0, 5);
	entities[5]->GetTransform()->MoveAbsolute(12, 0, 5);
	entities[6]->GetTransform()->MoveAbsolute(15, 0, 5);
	entities[7]->GetTransform()->MoveAbsolute(0, -3, 5);
	entities[8]->GetTransform()->MoveAbsolute(6, -3, 5);
	entities[9]->GetTransform()->MoveAbsolute(12, -3, 5);
	entities[10]->GetTransform()->MoveAbsolute(3, -3, 5);


	//lighting
	ambientColor = XMFLOAT3(0.1f, 0.1f, 0.25f); //(0.07f, 0.15f, 0.2f);

	//lights
	dirLight1 = {};
	dirLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight1.Direction = XMFLOAT3(1, 0, 0);
	dirLight1.Color = XMFLOAT3(1, 0, 0);
	dirLight1.Intensity = 1.0;

	Light dirLight2 = {};
	dirLight2.Color = XMFLOAT3(0, 0, 1);
	dirLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight2.Intensity = 0.5f;
	dirLight2.Direction = XMFLOAT3(0, 1, 0);

	Light dirLight3 = {};
	dirLight3.Color = XMFLOAT3(0, 1, 0);
	dirLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight3.Intensity = 0.5f;
	dirLight3.Direction = XMFLOAT3(0, 0, 1);

	Light pointLight1 = {};
	pointLight1.Color = XMFLOAT3(0, 1, 1);
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Intensity = 1.0f;
	pointLight1.Position = XMFLOAT3(-1.5f, 0, 0);
	pointLight1.Range = 10.0f;

	Light spotLight1 = {};
	spotLight1.Color = XMFLOAT3(1, 0, 1);
	spotLight1.Type = LIGHT_TYPE_SPOT;
	spotLight1.Intensity = 2.0f;
	spotLight1.Position = XMFLOAT3(6.0f, 1.5f, 0);
	spotLight1.Direction = XMFLOAT3(0, -1, 0);
	spotLight1.Range = 10.0f;
	spotLight1.SpotOuterAngle = XMConvertToRadians(30.0f);
	spotLight1.SpotInnerAngle = XMConvertToRadians(20.0f);


	lights.push_back(dirLight1);
	lights.push_back(dirLight2);
	lights.push_back(dirLight3);
	lights.push_back(pointLight1);
	lights.push_back(spotLight1);

	for (int i = 0; i < lights.size(); i++)
		if (lights[i].Type != LIGHT_TYPE_POINT)
			XMStoreFloat3(
				&lights[i].Direction,
				XMVector3Normalize(XMLoadFloat3(&lights[i].Direction))
			);

}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	//update proj matrix based on aspect ratio	
	float aspectRatio = Window::AspectRatio();
	for (auto& camera : cameras)
	{
		camera->UpdateProjectionMatrix(aspectRatio);
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//new frame init
	ImGuiFrame(deltaTime);
	//UI creation
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	//update entity transforms each frame
	//float scale = (float)sin(totalTime * 5) * 0.5f + 1.0f;
	//entities[0]->GetTransform()->SetScale(scale, scale, scale);

	if (cameras.size() > 0)
	{
		cameras[activeCameraIndex]->Update(deltaTime);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	colorPkr);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		//get matrices from the camera
		//copy data to constant buffer
		for (auto& entity : entities)
		{
			entity->GetMat()->GetPixelShader()->SetFloat("time", totalTime);
			entity->GetMat()->GetPixelShader()->SetFloat3("ambientColor", ambientColor);
			//entity->GetMat()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
			entity->GetMat()->GetPixelShader()->SetInt("lightCount", (int)lights.size());
			entity->GetMat()->GetPixelShader()->SetData(
				"lights", &lights[0], // The address of the data to set
				sizeof(Light) * (int)lights.size());// The address of the data to set
			entity->Draw(cameras[activeCameraIndex]);

		}
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

void Game::ImGuiFrame(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	//ImGui::ShowDemoWindow();
}

void Game::BuildUI()
{
	ImGui::Begin("Custom UI Window + Mesh Info"); // Everything after is part of the window
	{
		ImGui::PushItemWidth(200);
		ImGui::Text("This text is in the window");
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
		ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());

		ImGui::ColorEdit4("RGBA Background Color", colorPkr);

		//DEMO WINDOW BUTTON
		if (ImGui::Button("Toggle ImGui Demo Window"))
		{
			demoWindowShown = !demoWindowShown;
		}
		if (demoWindowShown)
		{
			ImGui::ShowDemoWindow();
		}

		//mesh ui info
		if (ImGui::CollapsingHeader("Mesh Debug Information"))
		{
			for (size_t i = 0; i < meshes.size(); i++)
			{
				auto& mesh = meshes[i];
				ImGui::Text("Mesh %zu: %s", i, meshes[i]->GetShapeName());
				ImGui::Text("Triangle Count: %d", (meshes[i]->GetIndexCount()/3));
				ImGui::Text("Vertex Count: %d", meshes[i]->GetVertexCount());
				ImGui::Text("Index Count: %d", meshes[i]->GetIndexCount());
				ImGui::Separator();
			}
		}

		//entity ui info
		if (ImGui::CollapsingHeader("Entity Debug Information"))
		{
			for (int i = 0; i < entities.size(); i++)
			{
				std::shared_ptr<Transform> trans = entities[i]->GetTransform();
				XMFLOAT3 pos = trans->GetPosition();
				XMFLOAT3 rot = trans->GetPitchYawRoll();
				XMFLOAT3 sca = trans->GetScale();
				XMFLOAT3 colorTint = entities[i]->GetMat()->GetColorTint();
				XMFLOAT2 uvScale = entities[i]->GetMat()->GetUVScale();
				XMFLOAT2 uvOffset = entities[i]->GetMat()->GetUVOffset();
				ImGui::PushID(i);
				if (ImGui::TreeNode("Entity Node", "Entity %d", i))
				{	
					ImGui::Text("Mesh Shape: %s", entities[i]->GetMesh()->GetShapeName());
					ImGui::Text("Material Name: %s", entities[i]->GetMat()->GetName());
					ImGui::Text("Index Count: %d", entities[i]->GetMesh()->GetIndexCount());
					if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) trans->SetPosition(pos);
					if (ImGui::DragFloat3("Rotation (Radians)", &rot.x, 0.01f)) trans->SetRotation(rot);
					if (ImGui::DragFloat3("Scale", &sca.x, 0.01f)) trans->SetScale(sca);

					//material color tint adjsut
					if (ImGui::ColorEdit4("Color Tint", &colorTint.x))
						entities[i]->GetMat()->SetColorTint(colorTint);

					//UV Scale and Offset Adjust
					if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.01f))
						entities[i]->GetMat()->SetUVScale(uvScale);
					if (ImGui::DragFloat2("UV Offset", &uvOffset.x, 0.01f))
						entities[i]->GetMat()->SetUVOffset(uvOffset);

					//for each of the entities textures [there is amax 2 right now]
					for (auto& it : entities[i]->GetMat()->GetTextureSRVMap())
					{
						//display texture name
						ImGui::Text(it.first.c_str());

						//if there is texutres
						if (it.second)
						{
							ImGui::Image((ImTextureID)it.second.Get(), ImVec2(256, 256));
						}
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		//mesh ui info
		if (ImGui::CollapsingHeader("Light Debug Information"))
		{
			//for each light
			for (int i = 0; i < lights.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::Text("Light: %d", i);
				std::string lightName = "Light %d";
				switch (lights[i].Type)
				{
				case LIGHT_TYPE_DIRECTIONAL: lightName += " (Directional)"; break;
				case LIGHT_TYPE_POINT: lightName += " (Point)"; break;
				case LIGHT_TYPE_SPOT: lightName += " (Spot)"; break;
				}

				//Color
				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				ImGui::SliderFloat("Intensity", &lights[i].Intensity, 0.0f, 10.0f);

				//Direction
				if (lights[i].Type == LIGHT_TYPE_DIRECTIONAL || lights[i].Type == LIGHT_TYPE_SPOT)
				{
					ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.1f);
					XMVECTOR dirNorm = XMVector3Normalize(XMLoadFloat3(&lights[i].Direction));
					XMStoreFloat3(&lights[i].Direction, dirNorm);
				}

				//Position/Range
				if (lights[i].Type == LIGHT_TYPE_POINT || lights[i].Type == LIGHT_TYPE_SPOT)
				{
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.1f);
					ImGui::SliderFloat("Range", &lights[i].Range, 0.1f, 100.0f);
				}


				

				ImGui::PopID();
			}
		}
	}
	ImGui::End();	

	ImGui::Begin("Camera Control");
	ImGui::PushItemWidth(200);
	for (int i = 0; i < cameras.size(); ++i)
	{
		//radio button label
		//swap between cameras at runtime
		std::string label = "Camera " + std::to_string(i + 1);
		if (ImGui::RadioButton(label.c_str(), activeCameraIndex == i))
		{
			activeCameraIndex = i; 
		}
	}

	//if there are cameras
	//display active camera details
	if (!cameras.empty())
	{
		auto& camera = cameras[activeCameraIndex];
		ImGui::Text("Active Camera: %d", activeCameraIndex + 1);
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera->GetTransform()->GetPosition().x,
			camera->GetTransform()->GetPosition().y, camera->GetTransform()->GetPosition().z);
		ImGui::Text("Field of View: %.2f", camera->Getfov());
	}
    ImGui::End();
}



