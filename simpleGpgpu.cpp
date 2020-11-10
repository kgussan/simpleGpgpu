#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <d3d11.h>
#include<DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")

// simpleBuffer 構造体
// CPUとGPUで同じフォーマット。好きな仕事をやらせるにはここを拡張してGPUに渡せばOK。
struct simpleBuffer_t
{
    int		i;
    float	f;
};

//コンスタントバッファ構造体
struct constBuf_t{
	uint32_t	numThres;	//処理内容を決める閾値
	uint32_t	numTotal;	//総数
	float		dummyVal;	//デバッグのために加算する値
	uint32_t	padding[5];
};//constant bufferはサイズが8の倍数でなければならない。


ID3D11Buffer	*g_pCB = NULL;

// 安全なリリース
void SafeRelease( IUnknown *p )
{
	if( p!=NULL ){
		p->Release();
	}
}

// 構造体バッファの生成
HRESULT CreateStructuredBufferOnGPU(	ID3D11Device	*pDevice, 
										UINT			uElementSize, 
										UINT			uCount, 
										void			*pInitData, 
										ID3D11Buffer	**ppBufOut )
{
    *ppBufOut = NULL;
    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof( desc ) );
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.ByteWidth = uElementSize * uCount;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = uElementSize;

    if( pInitData ){
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pInitData;
        return pDevice->CreateBuffer( &desc, &InitData, ppBufOut );
    } else {
        return pDevice->CreateBuffer( &desc, NULL, ppBufOut );
	}
}

// シェーダリソースビューを構造体バッファか、そのままのバッファのために生成
HRESULT CreateBufferSRV(	ID3D11Device				*pDevice, 
							ID3D11Buffer				*pBuffer, 
							ID3D11ShaderResourceView	**ppSRVOut )
{
    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof( descBuf ) );
    pBuffer->GetDesc( &descBuf );
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;

    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS ){
        // そのままのバッファ処理
        desc.Format					= DXGI_FORMAT_R32_TYPELESS;
        desc.BufferEx.Flags			= D3D11_BUFFEREX_SRV_FLAG_RAW;
        desc.BufferEx.NumElements	= descBuf.ByteWidth / 4;
    } else{
		if( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED ){
			// 構造体バッファ処理
			desc.Format					= DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements	= descBuf.ByteWidth / descBuf.StructureByteStride;
		}else{
			return E_INVALIDARG;
		}
	}
    return pDevice->CreateShaderResourceView( pBuffer, &desc, ppSRVOut );
}

// 構造体バッファ、またはそのままのバッファのためのUAVの生成
HRESULT CreateBufferUAV(	ID3D11Device				*pDevice, 
							ID3D11Buffer				*pBuffer, 
							ID3D11UnorderedAccessView	**ppUAVOut )
{
    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof( descBuf ) );
    pBuffer->GetDesc( &descBuf );
        
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof( desc ) );
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;

    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS ){
        desc.Format				= DXGI_FORMAT_R32_TYPELESS;
        desc.Buffer.Flags		= D3D11_BUFFER_UAV_FLAG_RAW;
        desc.Buffer.NumElements = descBuf.ByteWidth / 4; 
    } else {
		if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED ){
			desc.Format				= DXGI_FORMAT_UNKNOWN;
			desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride; 
		}else{
			return E_INVALIDARG;
		}
	}    
    return pDevice->CreateUnorderedAccessView( pBuffer, &desc, ppUAVOut );
}


// CPUがアクセスできるバッファを作成し、GPUバッファの内容をダウンロードする。
// この関数はCSプログラムのデバッグにとても便利
ID3D11Buffer* CreateAndCopyToCpuReadableMem(ID3D11Device		*pDevice, 
											ID3D11DeviceContext *pd3dImmediateContext, 
											ID3D11Buffer		*pBuffer )
{
	HRESULT hr;
    ID3D11Buffer *buf = NULL;

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof( desc ) );
    pBuffer->GetDesc( &desc );
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage			= D3D11_USAGE_STAGING;
    desc.BindFlags		= 0;
    desc.MiscFlags		= 0;
    hr = pDevice->CreateBuffer( &desc, NULL, &buf );
	if( FAILED( hr ) ){
		printf("failed %x\n", hr);
		return 0;
	}
    pd3dImmediateContext->CopyResource( buf, pBuffer );

    return buf;
}

HRESULT CreateComputeShader(	TCHAR				*csoName, 
								ID3D11Device		*pd3dDevice, 
								ID3D11ComputeShader	**resCS )
{
    //バイナリファイルを読み込む//
	FILE *fp;
	fopen_s( &fp, csoName, "rb");
	if( fp == NULL){
		return -1;//失敗
	}
	fseek(fp, 0, SEEK_END);		//終端まで行く。
	long cso_sz = ftell(fp);	//サイズを知る
	fseek(fp, 0, SEEK_SET);		//先頭に戻る
	
	unsigned char *cso_data = new unsigned char[ cso_sz ];
	fread( cso_data, cso_sz, 1, fp );    //コンパイル後のバイナリをcso_dataに格納
	fclose( fp );
	// シェーダーオブジェクトの作成
	ID3D11ComputeShader	*pComputeShader = NULL;
	HRESULT hr = pd3dDevice->CreateComputeShader( cso_data, cso_sz, NULL, &pComputeShader );

    *resCS = pComputeShader;
	delete [] cso_data;
    return hr;
}


// Compute Shader 実行
void Exec(void *)
{
	HRESULT hr;
	::std::vector<simpleBuffer_t> g_vSimpleBuf0;
	::std::vector<simpleBuffer_t> g_vSimpleBuf1;
	static const uint32_t kNumElements = 100;

	ID3D11DeviceContext *pContextOut	= NULL;
	ID3D11Device		*pDeviceOut		= NULL;
	D3D_FEATURE_LEVEL	flOut			= D3D_FEATURE_LEVEL_11_0;


	hr = D3D11CreateDevice(	
						NULL,
						D3D_DRIVER_TYPE_HARDWARE,
						NULL, 
						//D3D11_CREATE_DEVICE_DEBUG, 
						NULL, //余計なフラグを入れるとクラッシュしがち。
						NULL, 
						0,
						D3D11_SDK_VERSION, 
						&pDeviceOut, 
						&flOut, 
						&pContextOut );

	if(hr != S_OK){//error check
		exit(1);
	}

	//compute shader生成
	ID3D11ComputeShader *pComputeShaderSimple = NULL;
	if(1){
		//ファイルから生成
		ID3DBlob *pErrorBlob	= NULL;
		ID3DBlob *pBlob			= NULL;
		//ディレクトリ基点はproject ファイルパス
		HRESULT hr = D3DCompileFromFile(L"simpleGpgpu.hlsl", 
												nullptr, 
												nullptr,
												"main",
												"cs_5_0",
												NULL,
												NULL, 
												&pBlob, 
												&pErrorBlob);

		if(hr != S_OK){//error check
			exit(1);
		}

		if( pBlob==NULL ){
			printf( "shader load error.\n" );
			exit( 1 ); 
		}
		pDeviceOut->CreateComputeShader(	pBlob->GetBufferPointer(), 
											pBlob->GetBufferSize(), 
											NULL, 
											&pComputeShaderSimple );
		SafeRelease( pErrorBlob );
		SafeRelease( pBlob );
	}else{
		//シェーダバイナリから生成
		//事前の手間がかかるが、実行時のコンパイルのペナルティがない。
		HRESULT hr = CreateComputeShader( "simpleGpgpu.cfx",
											pDeviceOut, 
											&pComputeShaderSimple );
		if(hr & 0x8000000){//error check
			exit(1);
		}
	}
	//入力バッファを生成
	g_vSimpleBuf0.resize( kNumElements );
    g_vSimpleBuf1.resize( kNumElements );

	//適当な値を設定する。
    for ( int i = 0; i < kNumElements; ++i ) {
        g_vSimpleBuf0[ i ].i = i;
        g_vSimpleBuf0[ i ].f = (float)i;
        g_vSimpleBuf1[ i ].i = i*2;
        g_vSimpleBuf1[ i ].f = (float)( i*2.0f );
    }
	
	ID3D11Buffer *pBufin1	 = NULL;
	ID3D11Buffer *pBufin2	 = NULL;
	ID3D11Buffer *pBufOutput = NULL;
	
	CreateStructuredBufferOnGPU(	pDeviceOut, 
									sizeof(simpleBuffer_t), 
									(UINT)g_vSimpleBuf0.size(), 
									&g_vSimpleBuf0[0], 
									&pBufin1 );

    CreateStructuredBufferOnGPU(	pDeviceOut, 
									sizeof(simpleBuffer_t), 
									(UINT)g_vSimpleBuf1.size(), 
									&g_vSimpleBuf1[0], 
									&pBufin2 );

    CreateStructuredBufferOnGPU(	pDeviceOut, 
									sizeof(simpleBuffer_t), 
									(UINT)g_vSimpleBuf0.size(), 
									NULL,				
									&pBufOutput );
	
	ID3D11ShaderResourceView	*pSrvCalcIn1		= NULL;
	ID3D11ShaderResourceView	*pSrvCalcIn2		= NULL;
	ID3D11UnorderedAccessView	*pUavCalcOutput		= NULL;

	CreateBufferSRV( pDeviceOut, pBufin1,		&pSrvCalcIn1 );
    CreateBufferSRV( pDeviceOut, pBufin2,		&pSrvCalcIn2 );
    CreateBufferUAV( pDeviceOut, pBufOutput,	&pUavCalcOutput );
	
	//コンピュートシェーダの実行
	ID3D11ShaderResourceView *pShaderResourceViews[ 2 ] = { pSrvCalcIn1, pSrvCalcIn2 };
	pContextOut->CSSetShader( pComputeShaderSimple, NULL, 0 );
    pContextOut->CSSetShaderResources( 0, 2, pShaderResourceViews );
	ID3D11UnorderedAccessView *pUnorderedAccessViews[ 1 ] = { pUavCalcOutput };
    pContextOut->CSSetUnorderedAccessViews( 0, 
											1, 
											pUnorderedAccessViews, 
											(UINT *)pUavCalcOutput );
	
	ID3D11Buffer	*pConstantBufferCs	= NULL;		//コンスタントバッファ

	constBuf_t constBuf;
	constBuf.numTotal	= kNumElements;		
	constBuf.numThres	= 10;				//この数までは足し算、これ以降はデバッグ動作
	constBuf.dummyVal	= 90000.0f;			//デバッグ動作時に使うパラメタ
	void			*pConstantBufferData   = (void *)&constBuf;//コンスタントバッファ内容
	DWORD			constantBufferNumBytes = sizeof( constBuf_t );
    
    // Create the Const Buffer
    D3D11_BUFFER_DESC constant_buffer_desc;
    memset( &constant_buffer_desc, 0x0, sizeof(constant_buffer_desc) );
    constant_buffer_desc.ByteWidth = sizeof(constBuf_t);
    constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = 0;
	constant_buffer_desc.MiscFlags = 0;
	hr = pDeviceOut->CreateBuffer( &constant_buffer_desc, NULL, &g_pCB);
    if( FAILED( hr ) ){
    	printf("failed %x \n",hr);
		exit(1);
	}

    pContextOut->UpdateSubresource( g_pCB, 0, NULL, &constBuf, 0, 0 );
    pContextOut->CSSetConstantBuffers( 0, 1, &g_pCB );

	pContextOut->Dispatch( ( kNumElements + 63 )/64, 1, 1 ); //実行単位で切り上げ

    ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
    pContextOut->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, (UINT*)(&ppUAViewNULL) );

    ID3D11ShaderResourceView* ppSRVNULL[3] = { NULL, NULL, NULL };
    pContextOut->CSSetShaderResources( 0, 3, ppSRVNULL );
    pContextOut->CSSetConstantBuffers( 0, 0, NULL );

	//結果をGPUから読み込み、CPUから読む2つ目のバッファにダンプ
	ID3D11Buffer *outputBuffer = CreateAndCopyToCpuReadableMem( pDeviceOut, 
																pContextOut, 
																pBufOutput );

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	simpleBuffer_t *p;
    pContextOut->Map( outputBuffer, 0, D3D11_MAP_READ, 0, &MappedResource );
	p = (simpleBuffer_t*)MappedResource.pData;

	if( 1 ){ //debug : 結果の出力
		FILE *fp;
		fopen_s(&fp,"simpleGpgpu_output.txt","w");
		for( int i = 0; i < kNumElements; i++ ){
			fprintf( fp, "in0 : %d %f \n", g_vSimpleBuf0[i].i, g_vSimpleBuf0[i].f );
			fprintf( fp, "in1 : %d %f \n", g_vSimpleBuf1[i].i, g_vSimpleBuf1[i].f );
			
			uint32_t	resultI;
			float		resultF;
			
			if( (unsigned int)i < constBuf.numThres ){
				resultI = g_vSimpleBuf0[i].i + g_vSimpleBuf1[i].i;
				resultF	= sinf( g_vSimpleBuf0[i].f ) * cosf( g_vSimpleBuf1[i].f );
			} else {
				resultI = i;
				resultF	= (float)i + constBuf.dummyVal;
			}

			fprintf( fp, "CPU result : %d %f \n", resultI, resultF );
			fprintf( fp, "GPU result : %d %f \n", p[i].i, p[i].f );
			if( resultI==p[i].i && resultF==p[i].f ){
				fprintf( fp, "[OK] CPU result and GPU result are the same \n" );
			}else{
				fprintf( fp, "[NG] \n" );
			}
			fprintf( fp, "\n" );
		}
		fclose( fp );
	}

	//リソースの解放
	SafeRelease( outputBuffer );
	SafeRelease( pSrvCalcIn1 );
	SafeRelease( pSrvCalcIn2 );
	SafeRelease( pUavCalcOutput );

	SafeRelease( pBufin1 );
	SafeRelease( pBufin2 );
	SafeRelease( pBufOutput );

	SafeRelease( pComputeShaderSimple );

	SafeRelease( pContextOut );
	SafeRelease( pDeviceOut );
}

// エントリポイント
int main(void)
{
	Exec(0);
	return 0;
}

