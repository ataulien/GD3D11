#pragma once
#include <unordered_map>
#include <d3d11.h>

/** Struct holds initial shader data for load operation*/
struct ShaderInfo
{
public:
	std::string name;				//Shader's name, used as key in map
	std::string fileName;			//Shader's filename (without 'system\\GD3D11\\shaders\\')
	std::string type;				//Shader's type: 'v' vertexShader, 'p' pixelShader
	int layout;						//Shader's input layout
	std::vector<int> cBufferSizes;	//Vector with size for each constant buffer to be created for this shader
	std::vector<D3D10_SHADER_MACRO> shaderMakros;
	//Constructor
	ShaderInfo(std::string n, std::string fn, std::string t, int l, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>())
	{
		name = n;
		fileName = fn;
		type = t;
		layout = l;
		cBufferSizes = std::vector<int>();

		shaderMakros = makros;
	}

	//Constructor
	ShaderInfo(std::string n, std::string fn, std::string t, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>())
	{
		name = n;
		fileName = fn;
		type = t;
		layout = 0;
		cBufferSizes = std::vector<int>();
		
		shaderMakros = makros;
	}
};

class D3D11PShader;
class D3D11VShader;
class D3D11HDShader;

class D3D11ShaderManager
{
public:
	D3D11ShaderManager();
	~D3D11ShaderManager();

	/** Creates list with ShaderInfos */
	XRESULT Init();

	/** Loads/Compiles Shaderes from list */
	XRESULT LoadShaders();

	/** Deletes all shaders and loads them again */
	XRESULT ReloadShaders();

	/** Called on frame start */
	XRESULT OnFrameStart();

	/** Deletes all shaders */
	XRESULT DeleteShaders();

	/** Return a specific shader */
	D3D11VShader* GetVShader(std::string shader);
	D3D11PShader* GetPShader(std::string shader);
	D3D11HDShader* GetHDShader(std::string shader);

private:
	std::vector<ShaderInfo> Shaders;							//Initial shader list for loading
	std::unordered_map<std::string, D3D11VShader*> VShaders;
	std::unordered_map<std::string, D3D11PShader*> PShaders;
	std::unordered_map<std::string, D3D11HDShader*> HDShaders;

	/** Whether we need to reload the shaders next frame or not */
	bool ReloadShadersNextFrame;
};