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
	LoadShaders();
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

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);

		//CONSTANT BUFFER
		//creation
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.ByteWidth = (sizeof(ConstantBufferData) + 15) / 16 * 16; //multiple of 16
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;
		Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf());

		//binding the constant buffer
		Graphics::Context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	ImGui::StyleColorsDark();

	//create camera
	float aspectRatio = Window::AspectRatio();
	XMFLOAT3 persStartPos = { 0.0f, 0.0f, -5.0f };
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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//mesh verts
	//triangle
	Vertex triangleVertices[] =
	{
		{ XMFLOAT3(0.75f, 0.5f, 0.0f), XMFLOAT4(1, 0, 0, 1) }, //top
		{ XMFLOAT3(0.95f, 0.0f, 0.0f), XMFLOAT4(0, 1, 0, 1) }, //bottom_r
		{ XMFLOAT3(0.55f, 0.0f, 0.0f), XMFLOAT4(0, 0, 1, 1) }  //bottom_l
	};
	unsigned int triangleIndices[] = { 0, 1, 2 };

	//rectangle
	Vertex quadVertices[] = {
		{ XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1, 1, 0, 1)},  //top_l
		{ XMFLOAT3(0.5f, 0.5f, 0.0f), XMFLOAT4(1, 0, 1, 1)},   //top_r
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(0, 0, 1, 1)}, //bottom_l
		{ XMFLOAT3(0.5f, 0.0f, 0.0f), XMFLOAT4(0, 1, 0, 1)}   //bottom_r
	};
	unsigned int rectIndices[] = { 0, 1, 2, 1, 3, 2 };
	
	//pentagon
	Vertex pentagonVertices[] = {
		{ XMFLOAT3(-0.4f, 0.25f, 0.0f), XMFLOAT4(1, 0, 0, 1)},  //center
		{ XMFLOAT3(-0.4f, 0.5f, 0.0f), XMFLOAT4(1, 0, 0, 1)},  //top
		{ XMFLOAT3(-0.1f, 0.25f, 0.0f), XMFLOAT4(1, 0, 1, 1)},  //top_r
		{ XMFLOAT3(-0.25f, 0.0f, 0.0f), XMFLOAT4(0, 1, 0, 1)}, //bottom_r
		{ XMFLOAT3(-0.55f, 0.0f, 0.0f), XMFLOAT4(0, 0, 1, 1)}, //bottom_l
		{ XMFLOAT3(-0.7f, 0.25f, 0.0f), XMFLOAT4(1, 1, 0, 1)}  //top_l
	};
	unsigned int pentagonIndices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 1 };
	
	auto rect = std::make_shared<Mesh>(quadVertices, 4, rectIndices, 6, "Quadrilateral");
	auto triangle = std::make_shared<Mesh>(triangleVertices, 3, triangleIndices, 3, "Triangle");
	auto pentagon = std::make_shared<Mesh>(pentagonVertices, 6, pentagonIndices, 15, "Pentagon");

	// Store meshes in the vector
	meshes.push_back(triangle);
	meshes.push_back(rect);
	meshes.push_back(pentagon);

	//GAME ENTITIES
	std::shared_ptr<GameEntity> ge1 = std::make_shared<GameEntity>(triangle);
	std::shared_ptr<GameEntity> ge2 = std::make_shared<GameEntity>(rect);
	std::shared_ptr<GameEntity> ge3 = std::make_shared<GameEntity>(pentagon);
	std::shared_ptr<GameEntity> ge4 = std::make_shared<GameEntity>(triangle); //same mesh as ge1 and ge5
	std::shared_ptr<GameEntity> ge5 = std::make_shared<GameEntity>(triangle); //same mesh " " 

	//adjust transforms
	ge1->GetTransform()->Rotate(0, 0, 0.1f);
	ge3->GetTransform()->MoveAbsolute(-1.2f, -0.3f, 0.0f);
	ge4->GetTransform()->MoveAbsolute(-0.5f, 0.1f, 0.0f);
	ge5->GetTransform()->MoveAbsolute(0.1f, -1.0f, 0.0f);

	//add to entity vector to loop through for drawing purposes
	entities.push_back(ge1);
	entities.push_back(ge2);
	entities.push_back(ge3);
	entities.push_back(ge4);
	entities.push_back(ge5);
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
	float scale = (float)sin(totalTime * 5) * 0.5f + 1.0f;
	entities[0]->GetTransform()->SetScale(scale, scale, scale);
	entities[1]->GetTransform()->Rotate(0, 0, deltaTime * 1.0f);
	entities[2]->GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
	entities[3]->GetTransform()->Rotate(0, 0, deltaTime * -1.0f);
	entities[4]->GetTransform()->SetPosition((float)cos(totalTime), 0, 0);

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
			entity->Draw(constantBuffer, cameras[activeCameraIndex]);
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
				ImGui::PushID(i);
				if (ImGui::TreeNode("Entity Node", "Entity %d", i))
				{	
					ImGui::Text("Mesh Shape: %s", entities[i]->GetMesh()->GetShapeName());
					ImGui::Text("Index Count: %d", entities[i]->GetMesh()->GetIndexCount());
					if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) trans->SetPosition(pos);
					if (ImGui::DragFloat3("Rotation (Radians)", &rot.x, 0.01f)) trans->SetRotation(rot);
					if (ImGui::DragFloat3("Scale", &sca.x, 0.01f)) trans->SetScale(sca);
					ImGui::TreePop();
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



