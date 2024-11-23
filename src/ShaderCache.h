#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ShaderCache {
    protected:
    static ShaderCache* instance;
    Ref<CCDictionary> m_shaders;
    std::string m_vertex = R"(
				attribute vec4 a_position;							
				attribute vec2 a_texCoord;							
				attribute vec4 a_color;								
																	
				#ifdef GL_ES										
				varying lowp vec4 v_fragmentColor;					
				varying mediump vec2 v_texCoord;					
				#else												
				varying vec4 v_fragmentColor;						
				varying vec2 v_texCoord;							
				#endif												
																	
				void main()	{													
					gl_Position = CC_MVPMatrix * a_position;		
					v_fragmentColor = a_color;						
					v_texCoord = a_texCoord;						
				}													
			)";
    public:

    void init();
    void add(std::string name, CCGLProgram* program);
    CCGLProgram* getProgram(std::string name);
    std::string getVertex();
    void createShader(std::string name, std::string frag);


    static ShaderCache* get() {
        if (!instance) {
            instance = new ShaderCache();
            instance->init();
        }
        return instance;
    }

    void clearShaders(){
        m_shaders->removeAllObjects();
    }
};
