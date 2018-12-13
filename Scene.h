// Samy Mannane, Donovan Neveux

//Sample of code about a ray tracing algorithm
//These are blocks of code and not a complete class

// [...]

//***********************************************
//Calculating diffuse contribution for a pixel
//***********************************************

		RGBColor colorDiffuse(RayTriangleIntersection const & intersection, PointLight const & light, Ray const & ray)
		{
			RGBColor Idiffuse;

			// Variables
			RGBColor diffus = intersection.triangle()->material()->getDiffuse() //+ intersection.triangle()->material()->getEmissive();

			Math::Vector3f raySource = ray.source().normalized();
			Math::Vector3f view = intersection.intersection() - raySource;
			Math::Vector3f dirLight = intersection.intersection() - light.position();
			Math::Vector3f normal = intersection.triangle()->normal();

			// ----------
			// Normal interpolation

			// u and v: coordinates of intersection
			double u = intersection.uTriangleValue();
			double v = intersection.vTriangleValue();

			Math::Vector3f toward = intersection.triangle()->normal();
			Math::Vector3f interpolate_normal = intersection.triangle()->sampleNormal(u, v, toward);

			// Scalaire : NormaleIntersection ° VecteurDistanceLight

			float scalaire1 = interpolate_normal.normalized() * view.normalized();

			if (scalaire1 < 0) {
      
				//we get minus vector in order to be in the good subspace
				interpolate_normal = -interpolate_normal;
			}

			float scalaire2 = interpolate_normal.normalized() * dirLight.normalized();

			Idiffuse = diffus * light.color() *std::max((float)0, scalaire2) / (dirLight.norm());

			return Idiffuse;
		}


//[...]


//***********************************************
//Calculating specular contribution for a pixel
//***********************************************

		RGBColor colorSpecular(RayTriangleIntersection const & intersection, PointLight const & light, Ray const & ray)
		{
			RGBColor Ispecular;

			// Variables
			RGBColor specular = intersection.triangle()->material()->getSpecular() //+ intersection.triangle()->material()->getEmissive();

			Math::Vector3f raySource = ray.source().normalized();
			Math::Vector3f view = raySource - intersection.intersection();
			Math::Vector3f dirLight = light.position() - intersection.intersection();
			Math::Vector3f reflected = intersection.triangle()->reflectionDirection(ray);

			//float scalaire1 = view.normalized() * reflected.normalized();
			//float scalaire2 = dirLight.normalized() * reflected.normalized();


			Ispecular = specular * light.color() * pow(std::max((float)0, (scalaire1)), intersection.triangle()->material()->getShininess()) / (dirLight.norm());


			return Ispecular;
		}


//[...]

//***********************************************
// Determine if a pixel is in a shadow
//***********************************************

		bool isShadowed(RayTriangleIntersection const & intersection, int rand)
		{
			
				Math::Vector3f lPos = m_lights[rand].position();

				double u = intersection.uTriangleValue();
				double v = intersection.vTriangleValue();
				double t = intersection.tRayValue();
				Math::Vector3f shadowDir = intersection.intersection() - lPos;
				Ray r(lPos, shadowDir);
				CastedRay sRay(r);
        

				for (auto it = m_geometries.begin(), end = m_geometries.end(); it != end; ++it)
				{
					(*it).second.intersection(sRay);
				}

			return (sRay.validIntersectionFound() && sRay.intersectionFound().triangle() != intersection.triangle());
		}

//[...]

//***********************************************
// Calculating texture contribution for a pixel
//***********************************************

		RGBColor colorTexture(RayTriangleIntersection const & intersection) {
			
			RGBColor cTexture (0.7, 0.7, 0.7);

			// u and v coordinates of intersection
			double u = intersection.uTriangleValue();
			double v = intersection.vTriangleValue();
			 
			if (intersection.triangle()->material()->hasTexture()) {
				cTexture = intersection.triangle()->sampleTexture(u, v);
			}
			
			return cTexture;
		}

//[...]

//***********************************************
// Calculating final color for a pixel
//***********************************************		
    
		RGBColor color(RayTriangleIntersection const & intersection, Ray const & ray)
		{
			RGBColor result;
			
			for (int i = 0; i < m_lights.size(); i++) {

				int light_size = m_lights.size();
				int rand = std::rand() % light_size;

				if(!isShadowed(intersection, rand)){
					PointLight l = m_lights[rand];
					result = colorDiffuse(intersection, l, ray) * colorTexture(intersection) + colorSpecular(intersection, l, ray) * colorTexture(intersection);
				}
			}
			return result;
		}

//[...]


//***********************************************
// Sending a ray
//***********************************************

RGBColor sendRay(Ray const & ray, int depth, int maxDepth, int diffuseSamples, int specularSamples)
		{
			RGBColor result(0.0, 0.0, 0.0);

			if (depth < maxDepth) {
				
        //--------------------------------------------------------
				// Without BVH structure (very slow to render)
				//--------------------------------------------------------
        
				RayTriangleIntersection r = computeIntersection(ray);

				if(r.valid()){

					Ray rayReflective(r.intersection(), r.triangle()->reflectionDirection(ray).normalized());
          
					for (auto it = m_lights.begin(), end = m_lights.end(); it != end; ++it)
					{	
						result = result + color(r, ray);
					}
					result = result + r.triangle()->material()->getSpecular() * sendRay(rayReflective, depth + 1, maxDepth, diffuseSamples, specularSamples);
				//---------------------------------------------------------

				
				//---------------------------------------------------------
				// With the BVH structure (hundered times faster to render)
        //---------------------------------------------------------
        
				RayTriangleIntersection bbox = bvh->traversingBVH(ray, 0, std::numeric_limits<double>::max());
				if (bbox.valid()) {

					Ray rayReflective(bbox.intersection(), bbox.triangle()->reflectionDirection(ray).normalized());

					for (auto it = m_lights.begin(), end = m_lights.end(); it != end; ++it)
					{
						result = result + color(bbox, *it, ray);
					}
          
          //Sum every contributions
					result = result + bbox.triangle()->material()->getSpecular() * sendRay(rayReflective, depth + 1, maxDepth, diffuseSamples, specularSamples);
				
          //--------------------------------------------------------
				}
			}
			else {
				result = (0.0, 0.0, 0.0);
			}
			return result;
		}
