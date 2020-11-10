//CPU側のデータ型と合わせる必要がある。
struct SimpleBuffer_t
{
    int		i;
    float	f;
};

//グローバル扱い
cbuffer constBuf_t : register( b0 )
{
	uint	g_numThres;
	uint	g_numTotal;
	float	g_dummyVal;
	uint	g_padding[5];
};

StructuredBuffer<SimpleBuffer_t>	InBuf0 : register( t0 );
StructuredBuffer<SimpleBuffer_t>	InBuf1 : register( t1 );
RWStructuredBuffer<SimpleBuffer_t>	OutBuf : register( u0 );

[numthreads(64,1,1)]
void main( uint3 id : SV_DispatchThreadID )
{
	if( id.x < g_numTotal ){
		if( id.x < g_numThres ){
			OutBuf[ id.x ].i = InBuf0[ id.x ].i + InBuf1[ id.x ].i;
			OutBuf[ id.x ].f = sin( InBuf0[ id.x ].f ) * cos( InBuf1[ id.x ].f );
		} else {
			OutBuf[ id.x ].i = id.x;
			OutBuf[ id.x ].f = (float)id.x + g_dummyVal;
		}
	}	
	//debug
	//OutBuf[ id.x ].i = g_num;
	//OutBuf[ id.x ].f = g_dummyVal;
}
