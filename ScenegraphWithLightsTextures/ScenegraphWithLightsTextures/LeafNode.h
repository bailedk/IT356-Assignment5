#ifndef _LEAF_H_
#define _LEAF_H_
#include "node.h"
#include "Object.h"
#include "Material.h"
#include <stack>
#include <glm/gtc/matrix_transform.hpp>
#include "Texture.h"
#include "Light.h"
#include <algorithm>


class LeafNode : public Node
{
public:

	LeafNode(Object *instanceOf,Scenegraph *graph,string name="")
		:Node(graph,name)
	{
		this->instanceOf = instanceOf;
		//default material
		
		material.setAmbient(1.0f,0.8f,0.8f);
		material.setDiffuse(0.8f,0.8f,0.8f);
		material.setSpecular(0.8f,0.8f,0.8f);
		material.setShininess(50.0f);
		
		/*
		material.setAmbient(0.1f,0.025f,0.025f);
		material.setDiffuse(0.8f,0.2f,0.2f);
		material.setSpecular(0.8f,0.2f,0.2f);
		material.setShininess(50.0f);
		*/

		texture = NULL;

		//delete tex;
	}

	~LeafNode(void)
	{

	}

	Node *clone()
	{
		LeafNode *newclone = new LeafNode(instanceOf,scenegraph,name);
		newclone->setMaterial(material);

		return newclone;
	}

	virtual void draw(stack<glm::mat4> &modelView)
    {
		GLuint a;
        if (instanceOf!=NULL)
		{
			 //get the color
           // glm::vec4 color = material.getAmbient();
			mv = modelView.top();
            //set the color for all vertices to be drawn for this object
            //glUniform3fv(scenegraph->objectColorLocation,1,glm::value_ptr(color));
			glUniformMatrix4fv(scenegraph->modelviewLocation,1,GL_FALSE,glm::value_ptr(modelView.top()));
			glUniformMatrix4fv(scenegraph->normalMatrixLocation,1,GL_FALSE,glm::value_ptr(glm::transpose(glm::inverse(modelView.top()))));
			
			glUniform3fv(scenegraph->mat_ambientLocation,1,glm::value_ptr(material.getAmbient()));
			glUniform3fv(scenegraph->mat_diffuseLocation,1,glm::value_ptr(material.getDiffuse()));
			glUniform3fv(scenegraph->mat_specularLocation,1,glm::value_ptr(material.getSpecular()));
			glUniform1f(scenegraph->mat_shininessLocation,material.getShininess());

			if(texture != NULL) {
				glUniformMatrix4fv(scenegraph->texturematrixLocation,1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));
				glEnable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,texture->getTextureID());
				glUniform1i(scenegraph->textureLocation,0);
				scenegraph->textureLocation;
			}
			else {

			}

			a = glGetError();
			instanceOf->draw();        
			a = glGetError();
		}
    }

	virtual void drawBB(stack<glm::mat4>& modelView)
	{
		if (bbDraw)
		{
			glm::mat4 bbTransform;

			bbTransform = glm::translate(glm::mat4(1.0),0.5f*(minBounds+maxBounds)) * glm::scale(glm::mat4(1.0),maxBounds-minBounds);
			glm::vec4 color = glm::vec4(1,1,1,1);
			//set the color for all vertices to be drawn for this object
			glUniform3fv(scenegraph->objectColorLocation,1,glm::value_ptr(color));
			glUniformMatrix4fv(scenegraph->modelviewLocation,1,GL_FALSE,glm::value_ptr(modelView.top() * bbTransform));
			scenegraph->getInstance("box")->draw();        		
		}


	}
	
	virtual void updateBB()
	{
		minBounds = instanceOf->getMinimumWorldBounds().xyz();
		maxBounds = instanceOf->getMaximumWorldBounds().xyz();
	}
	virtual void getLights(vector<Light>& l, stack<glm::mat4>& modelView){
	
		for(int i =0; i<lights.size();i++){
			Light light = lights[i];
			glm::vec4 newPos= lights[i].getPosition();
			light.setPosition(glm::vec3(modelView.top()*lights[i].getPosition()));
			l.push_back(light);
		}
		
	}

	glm::vec4 getColor()
	{
		return material.getAmbient();
	}

	/*
		*Set the material of each vertex in this object
		*/
	virtual void setMaterial(Material& mat)
	{
		material = mat;
	}

	/*
		* gets the material
		*/
	Material getMaterial()
	{
		return material;
	}

	void setTexture(Texture *tex)
	{
		//cout << "Texture set to " << tex->getName() << endl;
		texture=tex;
	}

	virtual glm::mat4 getCameraTransform(){
		return glm::mat4(1.0);
	}

	virtual glm::mat4 getTransform(){
		return glm::mat4(1.0);
	}
	virtual bool intersect(Ray ray, Hit& hit,stack<glm::mat4>& modelView){
		//cout << "intersect in leaf" << endl;
		bool hasHit = false;

		glm::vec4 dir = glm::inverse(modelView.top())*ray.getDirection();
		glm::vec4 start = glm::inverse(modelView.top())*ray.getStart();
		if(instanceOf->getName().compare("sphere") ==0) {
			float a = dir.x * dir.x + dir.y * dir.y + dir.z *dir.z;
			float b = (2*dir.x*start.x)+(2*dir.y*start.y)+(2*dir.z*start.z);
			float c = start.x*start.x+start.y*start.y+start.z*start.z-1.0f;
			float iTest = b*b-4*a*c;
			if(iTest<0) {
				return false;
			} else {
				float tPos = (-1.0f*b+sqrt(iTest))/(2*a);
				float tNeg = (-1.0f*b-sqrt(iTest))/(2*a);

				float t = tNeg;
				if(tNeg<0){
					t = tPos;
				}
				if(tPos>0&&tPos<tNeg){
					t = tPos;
				}
				if(hit.getT()>t||hit.getT()<0){
					hit.setT(t);
					hit.setMat(material);
					glm::mat4 normalMatrix= glm::transpose(glm::inverse(modelView.top()));
					glm::vec4 norm = glm::vec4((start.x +(t*dir.x)), (start.y +(t*dir.y)), (start.z +(t*dir.z)), 0.0f);
					hit.setNormal(normalMatrix * glm::vec4(norm.x,norm.y,norm.z,0.0f)); 
					hit.setIntersection(modelView.top() * glm::vec4(norm.x,norm.y,norm.z,1.0f)); 
				}

				
				return true;
			}
		}
		// Need to add hit record stuff
		else if(instanceOf->getName().compare("box") == 0) {
			float tNear = FLT_MAX * -1;
 			float tFar = FLT_MAX;

			hit.setMat(material);

			// Uses Kay and Kayjia "Slab" Method found below
			// https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

			// x
			if (dir.x != 0.0) {
				float tx1 = (-0.5 - start.x)/dir.x;
				float tx2 = (0.5 - start.x)/dir.x;
 
				if(tx1 > tx2) {
					float temp;
					temp = tx1;
					tx1 = tx2;
					tx2 = temp;
				}
			
				if(tx1 > tNear) {
					tNear = tx1;
				}

				if(tx2 < tFar) {
					tFar = tx2;
				}

			}
 
			// y
			if (dir.y != 0.0) {
				float ty1 = (-0.5 - start.y)/dir.y;
				float ty2 = (0.5 - start.y)/dir.y;

				if(ty1 > ty2) {
					float temp;
					temp = ty1;
					ty1 = ty2;
					ty2 = temp;
				}
			
				if(ty1 > tNear) {
					tNear = ty1;
				}

				if(ty2 < tFar) {
					tFar = ty2;
				}
 
			}

			// z
			if (dir.z != 0.0) {
				float tz1 = (-0.5 - start.z)/dir.z;
				float tz2 = (0.5 - start.z)/dir.z;

				if(tz1 > tz2) {
					float temp;
					temp = tz1;
					tz1 = tz1;
					tz2 = temp;
				}
			
				if(tz1 > tNear) {
					tNear = tz1;
				}

				if(tz2 < tFar) {
					tFar = tz2;
				}

			}
 
			// i think there needs to be an extra check here, but won't be able to be sure until shade works
			if(tFar >= tNear) {
				hit.setT(tFar);
			}
			else {
				hit.setT(tNear);
			}

			return tFar >= tNear;

		}

		// if it's not an object we have just give a no-hit
		return false;
	}
	
protected:
	Object *instanceOf;
	Texture *texture;
	Material material;
};
#endif
