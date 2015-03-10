// Args
#define D3DTA_DIFFUSE           0x00000000  // select diffuse color
#define D3DTA_CURRENT           0x00000001  // select result of previous stage
#define D3DTA_TEXTURE           0x00000002  // select texture color
#define D3DTA_TFACTOR           0x00000003  // select RENDERSTATE_TEXTUREFACTOR

// Control
#define D3DTOP_DISABLE     1      // disables stage
#define D3DTOP_SELECTARG1  2      // the default
#define D3DTOP_SELECTARG2  3

// Modulate
#define D3DTOP_MODULATE    4      // multiply args together
#define D3DTOP_MODULATE2X  5      // multiply and  1 bit
#define D3DTOP_MODULATE4X  6      // multiply and  2 bits

// Add
#define D3DTOP_ADD            7   // add arguments together
#define D3DTOP_ADDSIGNED      8   // add with -0.5 bias
#define D3DTOP_ADDSIGNED2X    9   // as above but left  1 bit
#define D3DTOP_SUBTRACT      10   // Arg1 - Arg2 with no saturation
#define D3DTOP_ADDSMOOTH     11   // add 2 args subtract product
                            // Arg1 + Arg2 - Arg1*Arg2
                            //  Arg1 + (1-Arg1)*Arg2

// Linear alpha blend: Arg1*(Alpha) + Arg2*(1-Alpha)
#define D3DTOP_BLENDDIFFUSEALPHA     12 // iterated alpha
#define D3DTOP_BLENDTEXTUREALPHA     13 // texture alpha
#define D3DTOP_BLENDFACTORALPHA      14 // alpha from D3DRENDERSTATE_TEXTUREFACTOR
// Linear alpha blend with pre-multiplied arg1 input: Arg1 + Arg2*(1-Alpha)
#define D3DTOP_BLENDTEXTUREALPHAPM   15 // texture alpha
#define D3DTOP_BLENDCURRENTALPHA     16 // by alpha of current color

// Specular mapping
#define D3DTOP_PREMODULATE             17     // modulate with next texture before use
#define D3DTOP_MODULATEALPHA_ADDCOLOR  18     // Arg1.RGB + Arg1.A*Arg2.RGB
                                        // COLOROP only
#define D3DTOP_MODULATECOLOR_ADDALPHA  19     // Arg1.RGB*Arg2.RGB + Arg1.A
                                        // COLOROP only
#define D3DTOP_MODULATEINVALPHA_ADDCOLOR  20  // (1-Arg1.A)*Arg2.RGB + Arg1.RGB
                                        // COLOROP only
#define D3DTOP_MODULATEINVCOLOR_ADDALPHA  21  // (1-Arg1.RGB)*Arg2.RGB + Arg1.A
                                        // COLOROP only

// Bump mapping
#define D3DTOP_BUMPENVMAP            22 
#define D3DTOP_BUMPENVMAPLUMINANCE   23 
// This can do either diffuse or specular bump mapping with correct input.
// Performs the function (Arg1.R*Arg2.R + Arg1.G*Arg2.G + Arg1.B*Arg2.B)
// where each component has been scaled and offset to make it signed.
// The result is replicated into all four (including alpha) channels.
// This is a valid COLOROP only.
#define D3DTOP_DOTPRODUCT3           24

#define D3DTOP_FORCE_DWORD  0x7fffffff