���{�����t���V���v����GPGPU (hlsl�V�F�[�_) �T���v��
h26-09
kgussan@gmail.com
================================================================

�R���X�^���g�o�b�t�@���g�p����V���v����GPGPU�T���v��

���T�v
CPU��GPU�ŉ��Z�A��Z���s���B
�v�Z�I����ACPU�v�Z���ʂ�GPU�v�Z���ʂ��r����B
���ʂ�CPU�Ŏ擾���ăe�L�X�g�Ō��ʂ��o�́B

�w�肵��index �ȍ~�́A
���͂���index�ɒ萔�o�b�t�@�Ŏw�肵���������Z���ďo�́B

�����@
���ꂪ����Ό��\�y��GPGPU�������邩������Ȃ��Ǝv�����J���܂��B
DirectX11�����삷��PC�ł���Γ����܂��B
�����炭DirectX SDK ��Widows SDK�̓C���X�g�[�����Ă������ق����������Ǝv���܂��B
���C�g�����h�łł������Y���B

----------------------------------------------------------------
���ӎ���

��MIT���C�Z���X
�����̊ɂ����C�Z���X�̂���őI���B

�����f��
�v���O�����𓮍삳�������Ƃɂ���؂̐ӔC�𕉂��܂���B

----------------------------------------------------------------
�Q�l

���ȉ��̃\�[�X���Q�l�ɂ����B
Asif Bahrainwala�����CODE Project �̋L������
http://www.codeproject.com/Articles/42612/DirectX-Compute-Shaders
���̃\�[�X�̃��C�Z���X�͎��̂��̂ɏ�����B�v�񂷂�Ə��p���pOK�B�R�[�h�ۏ؂Ȃ��B
http://www.codeproject.com/info/cpol10.aspx

���m�F��
1)
Windows7
Visual Studio 2010
Microsoft DirectX SDK (June 2010)
2)
Windows8.1
Visual Studio 2010
Microsoft DirectX SDK (June 2010)
Windows SDK 8.1 �����Ӂ@windows 8.1�œ���ɕK�{

��HLSL�V�F�[�_�R���p�C��
�R���s���[�g�V�F�[�_�͎��s���x����̂��߁A�]���̌݊����̎���`�ɂ��邽�߁A
�I�t���C���R���p�C�����邱�Ƃ��ł���B
fxc �R���p�C���Ŏ��̂悤�ɂ��ăo�C�i�������
> $(DXSDK_DIR)/Utilities/bin/x64/fxc /T cs_5_0 /Fo simpleGpgpu.cfx simpleGpgpu.hlsl 

$(DXSDK_DIR)�̗�
	c:\Program Files (x86)\Microsoft DirectX SDK (June 2010)
��Lbin �p�X�����ϐ��Ɏw�肷��ƕ֗��B
�Q�l�F	http://marupeke296.com/DXPS_No10_CreatePricompileFXFile.html
