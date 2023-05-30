// ╔═══════════════════════════════════╗
// ║     Normals                       ║
// ╚═══════════════════════════════════╝
// sourced and modified from http://www.thetenthplanet.de/archives/1180

#ifdef SPR_NORMALS
  mat3 inverse3x3( mat3 M ){
      mat3 M_t = transpose( M ); 
      float det = dot( cross( M_t[0], M_t[1] ), M_t[2] ); 
      mat3 adjugate = mat3( 
          cross( M_t[1], M_t[2] ), 
          cross( M_t[2], M_t[0] ), 
          cross( M_t[0], M_t[1] ) ); 
      return adjugate / det;
  }

  mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv ) { 
      // get edge vectors of the pixel triangle 
      vec3 dp1 = dFdx( p ); 
      vec3 dp2 = dFdy( p ); 
      vec2 duv1 = dFdx( uv ); 
      vec2 duv2 = dFdy( uv );   
      // solve the linear system 
      vec3 dp2perp = cross( dp2, N ); 
      vec3 dp1perp = cross( N, dp1 ); 
      vec3 T = dp2perp * duv1.x + dp1perp * duv2.x; 
      vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;   
      // construct a scale-invariant frame 
      float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) ); 
      return mat3( T * invmax, B * invmax, N ); 
  }

  vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord, vec3 mapNormal ){
      mapNormal.y = -mapNormal.y;
      mat3 TBN = cotangent_frame( N, -V, texcoord );
      return normalize( TBN * mapNormal );
  }
#endif



// ╔═══════════════════════════════════╗
// ║     Blur                          ║
// ╚═══════════════════════════════════╝
// sourced and modified from https://github.com/Jam3/glsl-fast-gaussian-blur

#ifdef SPR_BLUR
vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction){
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3333333333333333) * direction;
  color += texture(image, uv) * 0.29411764705882354;
  color += texture(image, uv + (off1 / resolution)) * 0.35294117647058826;
  color += texture(image, uv - (off1 / resolution)) * 0.35294117647058826;
  return color; 
}

vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += texture(image, uv) * 0.2270270270;
  color += texture(image, uv + (off1 / resolution)) * 0.3162162162;
  color += texture(image, uv - (off1 / resolution)) * 0.3162162162;
  color += texture(image, uv + (off2 / resolution)) * 0.0702702703;
  color += texture(image, uv - (off2 / resolution)) * 0.0702702703;
  return color;
}

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture(image, uv) * 0.1964825501511404;
  color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}
#endif



// ╔═══════════════════════════════════╗
// ║     Random                        ║
// ╚═══════════════════════════════════╝
// sourced from https://stackoverflow.com/users/2434130/spatial

#ifdef SPR_RANDOM
  uint hash(uint x){
      x += ( x << 10u );
      x ^= ( x >>  6u );
      x += ( x <<  3u );
      x ^= ( x >> 11u );
      x += ( x << 15u );
      return x;
  }

  uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
  uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
  uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

  // Construct a float with half-open range [0:1] using low 23 bits.
  // All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
  float floatConstruct(uint m){
      const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
      const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

      m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
      m |= ieeeOne;                          // Add fractional part to 1.0

      float  f = uintBitsToFloat( m );       // Range [1:2]
      return f - 1.0;                        // Range [0:1]
  }

  // Pseudo-random value in half-open range [0:1].
  float random(float x) {return floatConstruct(hash(floatBitsToUint(x)));}
  float random(vec2  v) {return floatConstruct(hash(floatBitsToUint(v)));}
  float random(vec3  v) {return floatConstruct(hash(floatBitsToUint(v)));}
  float random(vec4  v) {return floatConstruct(hash(floatBitsToUint(v)));}
#endif