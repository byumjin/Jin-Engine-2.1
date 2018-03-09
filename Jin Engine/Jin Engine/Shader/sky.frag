#version 450
#extension GL_ARB_separate_shader_objects : enable

#define DEGREE_TO_RADIAN 0.01745329251994329576923690768489
#define EARTH_RADIUS 2000000.0
#define CLOUD_MIN 1500.0
#define CLOUD_MAX 8000.0

#define PI 3.1415926535897932384626422832795028841971
#define TwoPi 6.28318530717958647692
#define InvPi 0.31830988618379067154
#define Inv2Pi 0.15915494309189533577
#define Inv4Pi 0.07957747154594766788

layout(binding = 0) uniform sampler2D SceneTexture;
layout(binding = 1) uniform sampler3D lowFreqTexture;
layout(binding = 2) uniform sampler3D highFreqTexture;
layout(binding = 3) uniform sampler2D wheatherTexture;

layout(set = 0, binding = 4) uniform cameraBuffer
{
	mat4 viewMat;
	mat4 projMat;
	mat4 viewProjMat;
	mat4 InvViewProjMat;

	vec4 cameraWorldPos;
};

layout(set = 0, binding = 5) uniform perFrameBuffer
{
	vec4 timeInfo;
};

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

struct Intersection {
    vec3 normal;
    vec3 point;
    bool valid;
    float t;
};

// Compute sphere intersection

Intersection raySphereIntersection(in vec3 ro, in vec3 rd, in vec4 sphere)
{
	Intersection result;
	result.t = -1.0;
	result.valid = false;

	// - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    vec3 s0_r0 = ro;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sphere.w * sphere.w);

	float discriminant = b*b - 4.0*a*c;

    if( discriminant < 0.0)
	{
        return result;
    }

    float t = (-sqrt(discriminant) - b) / a * 0.5;
    if (t < 0.0) t = (sqrt(discriminant) - b) / a * 0.5;

	result.t = t;
	result.valid = true;
	return result;
}

/*
Intersection raySphereIntersection(in vec3 ro, in vec3 rd, in vec4 sphere) {
	Intersection isect;
    isect.valid = false;
    isect.point = vec3(0);
    isect.normal = vec3(0, 1, 0);
    
    // no rotation, only uniform scale, always a sphere
    ro -= sphere.xyz;
    ro /= sphere.w;
    
    float A = dot(rd, rd);
    float B = 2.0 * dot(rd, ro);
    float C = dot(ro, ro) - 0.25;
    float discriminant = B * B - 4.0 * A * C;
    
    if (discriminant < 0.0) return isect;
    float t = (-sqrt(discriminant) - B) / A * 0.5;
    if (t < 0.0) t = (sqrt(discriminant) - B) / A * 0.5;
    
    if (t >= 0.0) {
        isect.valid = true;
    	vec3 p = vec3(ro + rd * t);
        isect.normal = normalize(p);
        p *= sphere.w;
        p += sphere.xyz;
        isect.point = p;
        isect.t = length(p - ro);
    }
    
    return isect;
}
*/

// fractional value for sample position in the cloud layer
float GetHeightFractionForPoint(vec3 inPosition, vec2 inCloudMinMax)
{
    // get global fractional position in cloud zone
    float height_fraction = (inPosition.y - inCloudMinMax.x )/(inCloudMinMax.y - inCloudMinMax.x);
    return clamp(height_fraction, 0.0, 1.0);
}


float Remap(float value, float original_min, float original_max, float new_min, float new_max)
{
    return new_min + ((value - original_min) / (original_max - original_min)) * (new_max - new_min);
}

float stratus(float height)
{
	return Remap(height, 0.05, 0.1, 0.0, 1.0) * Remap(height, 0.2, 0.25, 1.0, 0.0);
}

float stratoCumulus(float height)
{
	return Remap(height, 0.1, 0.2, 0.0, 1.0) * Remap(height, 0.35, 0.45, 1.0, 0.0);
}

float cumulus(float height)
{
	return Remap(height, 0.05, 0.1, 0.0, 1.0) * Remap(height, 0.7, 0.8, 1.0, 0.0);
}

float getCloudType(vec3 weatherData)
{
	// weather b channel tells the cloud type 0.0 = stratus, 0.5 = stratocumulus, 1.0 = cumulus
	return weatherData.b;
}

vec4 mixGradients(float cloudType)
{
	vec4 STRATUS_GRADIENT = vec4(0.00f, 0.02f, 0.09f, 0.11f);
	vec4 STRATOCUMULUS_GRADIENT = vec4(0.02f, 0.2f, 0.48f, 0.625f);
	vec4 CUMULUS_GRADIENT = vec4(0.01f, 0.0625f, 0.78f, 1.0f);

	float stratus = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
	float stratocumulus = 1.0 - abs(cloudType - 0.5) * 2.0;
	float cumulus = clamp(cloudType - 0.5, 0.0, 1.0) * 2.0;
	return STRATUS_GRADIENT * stratus + STRATOCUMULUS_GRADIENT * stratocumulus + CUMULUS_GRADIENT * cumulus;
}

float GetDensityHeightGradientForPoint(float heightFraction, vec3 weatherData)
{
	//vec4 cloudGradient = mixGradients(getCloudType(weatherData));
	//return smoothstep(cloudGradient.x, cloudGradient.y, heightFraction) - smoothstep(cloudGradient.z, cloudGradient.w, heightFraction);

	if(weatherData.z < 0.5)
	{
		return stratus(heightFraction);
	}
	else if(weatherData.z < 1.0)
	{
		return stratoCumulus(heightFraction);
	}
	else if(weatherData.z >= 0.5)
	{
		return cumulus(heightFraction);
	}
}

float SphericalTheta(vec3 v)
{
	return acos(clamp(v.y, -1.0f, 1.0f));
}

float SphericalPhi(vec3 v)
{
	float p = atan(v.z , v.x);
	return (p < 0.0f) ? (p + TwoPi) : p;
}

float SampleCloudDensity(vec3 position, float height_fraction, vec3 weather_data, bool doCheaply)
{
	vec2 inCloudMinMax = vec2(CLOUD_MIN, CLOUD_MAX);

	vec3 normalizePos = normalize(position);
	vec3 planetUp = vec3(0.0, 1.0, 0.0);

	if( dot(normalizePos, planetUp) > 0.99 )
	{
		planetUp = vec3(0.0, 0.0, 1.0);
	}

	vec3 planetRight = cross(normalizePos, planetUp);
	planetUp =  cross(planetRight, normalizePos);

	vec3 newPosition;
	newPosition = position;

	//newPosition.x = dot(position, planetRight);
	//newPosition.y = (CLOUD_MAX - CLOUD_MIN) * height_fraction + CLOUD_MIN;
	//newPosition.z = dot(position, planetUp);

    // get height fraction  (be sure to create a cloud_min_max variable)
    //float height_fraction = GetHeightFractionForPoint(newPosition, inCloudMinMax);
    
    // wind settings
    vec3 wind_direction = vec3(1.0, 0.0, 0.0);
    float cloud_speed = 10.0;

    // cloud_top offset - push the tops of the clouds along this wind direction by this many units.
    float cloud_top_offset = 500.0;

    // skew in wind direction
    //position += height_fraction * wind_direction * cloud_top_offset;

    //animate clouds in wind direction and add a small upward bias to the wind direction
    //position += (wind_direction + vec3(0.0, 0.1, 0.0)  ) * timeInfo.x * cloud_speed;

	float scale = 0.000057;
	newPosition *= scale;
    // read the low frequency Perlin-Worley and Worley noises
    vec4 low_frequency_noises = texture(lowFreqTexture, newPosition);

    // build an fBm out of  the low frequency Worley noises that can be used to add detail to the Low frequency Perlin-Worley noise
    float low_freq_fBm = ( low_frequency_noises.g * 0.625 ) + ( low_frequency_noises.b * 0.25 ) + ( low_frequency_noises.a * 0.125 );

    // define the base cloud shape by dilating it with the low frequency fBm made of Worley noise.
    float base_cloud = Remap( low_frequency_noises.r, low_freq_fBm - 1.0, 1.0, 0.0, 1.0 );

    // Get the density-height gradient using the density-height function (not included)
    float density_height_gradient = GetDensityHeightGradientForPoint(height_fraction, weather_data);

    // apply the height function to the base cloud shape
    base_cloud *=  density_height_gradient;

    // cloud coverage is stored in the weather_data’s red channel.
    float cloud_coverage = weather_data.r;

	//float anvil_bias = 0.4;

    // apply anvil deformations
    //cloud_coverage = pow(cloud_coverage, Remap(height_fraction, 0.7, 0.8, 1.0, mix(1.0, 0.5, anvil_bias)));

    //Use remapper to apply cloud coverage attribute
    float base_cloud_with_coverage  = Remap(base_cloud, cloud_coverage, 1.0, 0.0, 1.0);
	//float base_cloud_with_coverage = base_cloud;


    //Multiply result by cloud coverage so that smaller clouds are lighter and more aesthetically pleasing.
    base_cloud_with_coverage *= cloud_coverage;

    //define final cloud value
    float final_cloud = base_cloud_with_coverage;

    // only do detail work if we are taking expensive samples!
    if(!doCheaply)
    {
        // add some turbulence to bottoms of clouds using curl noise.  Ramp the effect down over height and scale it by some value (200 in this example)
        //vec2 curl_noise = texture(Cloud2DNoiseTexture,  vec4 (vec2(newPosition.xyy), 0.0, 1.0).rg);
        //newPosition.xy += curl_noise.rg * (1.0 - height_fraction) * 200.0;

        // sample high-frequency noises
        vec3 high_frequency_noises = texture(highFreqTexture, newPosition * 0.1).rgb;

        // build High frequency Worley noise fBm
        float high_freq_fBm = ( high_frequency_noises.r * 0.625 ) + ( high_frequency_noises.g * 0.25 ) + ( high_frequency_noises.b * 0.125 );

        // get the height_fraction for use with blending noise types over height
        //float height_fraction  = GetHeightFractionForPoint(newPosition, inCloudMinMax);

        // transition from wispy shapes to billowy shapes over height
        float high_freq_noise_modifier = mix(high_freq_fBm, 1.0 - high_freq_fBm, clamp(height_fraction * 10.0, 0.0, 1.0));

        // erode the base cloud shape with the distorted high frequency Worley noises.
        final_cloud = Remap(base_cloud_with_coverage, high_freq_noise_modifier * 0.2 , 1.0, 0.0, 1.0);
    }

    return clamp(final_cloud, 0.0, 1.0);
}

mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

vec2 noise_kernel[7] = { vec2( 60.586189879377, 39.945621839699 ),
						vec2( 260.14234169393, 48.831649678215 ),
						vec2( 128.89511447814, 60.596356750744 ),
						vec2( 19.962449399737, 73.639731115494 ),
						vec2( 305.41964960537, 84.283728070689 ),
						vec2( 157.32709465424, 61.401314624306 ),
						vec2( 171.65412551335, 18.212465261208 ) };

vec3 getUniformConeVector(vec3 lightVec, float half_cone, int index)
{
	float z = sin( Remap(noise_kernel[index].y, 0.0, 90.0, 90.0 - half_cone, 90.0 ) * DEGREE_TO_RADIAN );
	float zP = sqrt(1.0 - z*z);
	float cosPhi = cos(noise_kernel[index].x  * DEGREE_TO_RADIAN);
	float sinPhi = sin(noise_kernel[index].x  * DEGREE_TO_RADIAN);


	vec3 randomVec = normalize(vec3(zP * cosPhi,  zP * sinPhi, z ));
	vec3 rotVec = cross(randomVec, lightVec);

	return rotationMatrix(rotVec, dot(rotVec, randomVec)) * randomVec;	
}


// a function to gather density in a cone for use with lighting clouds.
float SampleCloudDensityAlongCone(vec3 p, vec3 ray_direction, vec3 weather_data, float light_step)
{
	// How wide to make the cone.
	float cone_spread_multiplier = length(light_step);
    float density_along_cone = 0.0;

	vec3 position;

    // lighting ray march loop
    for(int i=1; i<=6; i++)
    {
        // add the current step offset to the sample position
        //p += light_step + ( cone_spread_multiplier * float(i) * getUniformConeVector(ray_direction, 30.0, i) );
		position = getUniformConeVector(ray_direction, 30.0, i) * float(i) * light_step + p;
        // sample cloud density the expensive way
        //int mip_offset = int(i * 0.5);

		//get projected Pos in innerShell;
		float relativeHeight = distance(position, normalize(position) * CLOUD_MIN) / (CLOUD_MAX - CLOUD_MIN);

        density_along_cone += SampleCloudDensity(position, relativeHeight, weather_data, false);
    }
	return clamp(density_along_cone, 0.0, 1.0);
}



// dl is the density sampled along the light ray for the given sample position.
// ds_loded is the low lod sample of density at the given sample position.

// get light energy
float GetLightEnergy(vec3 p, float height_fraction, float dl, float ds_loded, float phase_probability, float cos_angle, float step_size, float brightness)
{
    // attenuation – difference from slides – reduce the secondary component when we look toward the sun.
    float primary_attenuation = exp( -dl );
    float secondary_attenuation = max(primary_attenuation, exp(-dl * 0.25) * 0.7);
    //float attenuation_probability = max( Remap( cos_angle, 0.7, 1.0, secondary_attenuation, secondary_attenuation * 0.25) , primary_attenuation);
	float attenuation_probability = mix(primary_attenuation, secondary_attenuation, -cos_angle * 0.5 + 0.5);

    // in-scattering – one difference from presentation slides – we also reduce this effect once light has attenuated to make it directional.
    float depth_probability = mix( 0.05 + pow( ds_loded, Remap( height_fraction, 0.3, 0.85, 0.5, 2.0 )), 1.0, clamp( dl / step_size, 0.0, 1.0));
    float vertical_probability = pow( Remap( height_fraction, 0.07, 0.14, 0.1, 1.0 ), 0.8 );
    float in_scatter_probability = depth_probability * vertical_probability;

    float light_energy = attenuation_probability;// * in_scatter_probability * brightness * phase_probability;

    return light_energy;
}


float HenyeyGreenstein(float cos_angle, float inG)
{	
	float inG2 = inG*inG;
	return ((1.0 - inG2) / pow((1.0 + inG2 - 2.0 * inG * cos_angle), 1.5)) / 12.566370614359172953850573533118;
}

void main()
{
	vec4 sceneColor = texture(SceneTexture, fragUV);
	float depth = sceneColor.w;
	float energy = 0.0;
	vec4 skyColor = vec4(0.0, 0.0, 0.0, 1.0);


	float isSky = 0.0;

	vec4 screenSpaceDireciton = InvViewProjMat * vec4(fragUV * 2.0 - vec2(1.0), 1.0, 1.0);
		screenSpaceDireciton /= screenSpaceDireciton.w;
	
	vec3 viewVec = normalize(screenSpaceDireciton.xyz - cameraWorldPos.xyz);

	vec3 lightVec = normalize(vec3(1.0, 4.0, 1.0));
	float LoV = dot(lightVec, viewVec);
	float HG = HenyeyGreenstein(LoV, 0.6);

	vec2 inCloudMinMax = vec2(CLOUD_MIN, CLOUD_MAX);

	vec4 atmosphereSphereInner = vec4(0.0, 0.0, 0.0, CLOUD_MIN + EARTH_RADIUS);
    vec4 atmosphereSphereOuter = vec4(0.0, 0.0, 0.0, CLOUD_MAX + EARTH_RADIUS);

	vec3 cameraPos =  cameraWorldPos.xyz + vec3(0.0, EARTH_RADIUS, 0.0);

	Intersection atmosphereIsectInner = raySphereIntersection(cameraPos, viewVec, atmosphereSphereInner);
    Intersection atmosphereIsectOuter = raySphereIntersection(cameraPos, viewVec, atmosphereSphereOuter);

	/*
	if(atmosphereIsectInner.valid == true)
	{
		outColor = vec4(1.0, 0.5, 0.5, 1.0);
		return;
	}
	

	if(atmosphereIsectOuter.valid == true)
	{
		outColor = vec4(0.5, 1.0, 0.0, 1.0);
		return;
	}
	*/

	float ds = 0.0;
	float cloud_test = 0.0;
	int zero_density_sample_count = 0;
	float sampled_density_previous = -1.0;
	float alpha = 0.0;


	float stepSize = 64.0;

	//sky
	if(depth >= 1.0)
	{		
		isSky = 1.0;

		vec3 weatherData = vec3(0.0); 				
		vec3 position;
		float step =  (CLOUD_MAX - CLOUD_MIN) / stepSize;

		for(float t = atmosphereIsectInner.t; t < atmosphereIsectOuter.t; ) 
		{
			float rHeight = (t - atmosphereIsectInner.t) / (atmosphereIsectOuter.t - atmosphereIsectInner.t);
			position = cameraPos + viewVec*t;

			if(alpha <= 1.0)
			{
				weatherData = texture(wheatherTexture, vec2(position.x, position.z) * 0.001).xyz;
				//weatherData.x -= 0.2;
				weatherData.z = 0.0;

				//weatherData.x = 0.999;
				weatherData.x = 0.01;

				//weatherData = clamp(weatherData, 0.0, 1.0);

				if(cloud_test > 0.0)
				{
					float sampled_density = SampleCloudDensity(position, rHeight, weatherData, true);

					if(sampled_density == 0.0 && sampled_density_previous == 0.0)
					{
						zero_density_sample_count++;
					}

					if(zero_density_sample_count < 11 && sampled_density > 0.0)
					{
						ds += sampled_density;
						//float dl = SampleCloudDensityAlongCone(position, lightVec, weatherData, step);
						energy += sampled_density * 0.5;// GetLightEnergy(position, rHeight, dl, sampled_density, HG, LoV, 7.0, 1.0) * sampled_density;
						alpha += sampled_density;
								
						
						t += step;
					}					
					else
					{
						cloud_test = 0.0;
						zero_density_sample_count = 0;
					}

					sampled_density_previous = sampled_density;
				}
				else
				{
					cloud_test = SampleCloudDensity(position, rHeight, weatherData, true);
					if(cloud_test <= 0.0)
					{
						t += step * 2.0;
					}
					else
					{
						t += step;
					}
				}
			}
			else
				break;
		}

		vec3 endWorldPos = viewVec * 1000.0 + cameraPos;

		vec3 midDayCol = vec3(0.52941176470588235294117647058824, 0.81176470588235294117647058823529, 0.90980392156862745098039215686275);
		vec3 midDayCol2 = vec3(0.94901960784313725490196078431373, 0.9, 0.76862745098039215686274509803922);
		
		skyColor.xyz = midDayCol;// mix(midDayCol2, midDayCol, Remap( endWorldPos.y, 0.0 , 500.0 , 0.0 , 1.0 ) );
	}

	//sceneColor = sceneColor + skyColor;

	vec4 cloudColor = vec4(energy, energy, energy, alpha);

	skyColor = mix(skyColor, cloudColor, cloudColor.a);

	outColor = mix(sceneColor, skyColor, isSky);
		
}
