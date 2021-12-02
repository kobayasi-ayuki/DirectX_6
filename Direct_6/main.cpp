// Include
#include <vector>
#include "DxLib.h"

// �e�N�X�`�����W�ۑ��p�\����
struct VECTOR2
{
    float x, y;
};

// �}�e���A���i�[�p
struct Material
{
    COLOR_U8 diffuse;   // �f�B�t���[�Y�J���[
    COLOR_U8 specular;  // �X�y�L�����J���[
    int texture;        // �O���t�B�b�N�n���h��
};

// WinMain
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpcmdLine, _In_ int nCmdShow)
{
    ChangeWindowMode(true);            // �E�B���h�E���[�h�ɐݒ�
    SetBackgroundColor(100, 149, 237); // �w�i�J���[

    // DX���C�u�����̏�����
    if (DxLib_Init() == -1)
    {
        // �G���[
        return 0;
    }

    // ����ʂɕ`�悷��
    SetDrawScreen(DX_SCREEN_BACK);

    // ���C�g����
    SetUseLighting(true);

    // Z�o�b�t�@�L��
    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(true);

    // �J�������W�ƒ����_�̐ݒ�
    SetCameraPositionAndTarget_UpVecY(
        VGet(0.0F, 1.0F, -1.0F), VGet(0.0F, 0.0F, 0.0F));

    // �J������NEAR��FAR�̐ݒ�
    SetCameraNearFar(0.5f, 100.0F);

    // �t�@�C�����J��
    FILE* fp = fopen("Chips.obj", "r");

    // NULL�`�F�b�N
    if (!fp)
    {
        // �G���[
        return 0;
    }

    // ���_���W�A�e�N�X�`�����W�A�@���x�N�g���i�[��̗p��
    std::vector<VECTOR> position;
    std::vector<VECTOR> normal;
    std::vector<VECTOR2> uv;
    Material material;

    while (!feof(fp))
    {
        char key[MAX_PATH] = "";

        // �L�[���[�h�ǂݍ���
        fscanf(fp, "%s ", key);

        // �L�[���[�h��"mtllib"��
        if (!strcmp(key, "mtllib"))
        {
            // �}�e���A���t�@�C���̓ǂݍ���
            fscanf(fp, "%s ", key);
            FILE* matfp = fopen(key, "r");

            while (!feof(matfp))
            {
                // �L�[���[�h�ǂݍ���
                fscanf(matfp, "%s ", key);

                // �L�[���[�h��"Kd"��
                if (!strcmp(key, "Kd"))
                {
                    // �f�B�t���[�Y�J���[�̓ǂݍ���
                    COLOR_F color;
                    fscanf(matfp, "%f %f %f", &color.r, &color.g, &color.b);

                    // 0�`255�ɕϊ����Ċi�[
                    material.diffuse.r = static_cast<BYTE>(color.r * 255.0F);
                    material.diffuse.g = static_cast<BYTE>(color.g * 255.0F);
                    material.diffuse.b = static_cast<BYTE>(color.b * 255.0F);
                    material.diffuse.a = 255;
                }

                // �L�[���[�h��"Ks"��
                if (!strcmp(key, "Ks"))
                {
                    // �X�y�L�����J���[�̓ǂݍ���
                    COLOR_F color;
                    fscanf(matfp, "%f %f %f", &color.r, &color.g, &color.b);
                    material.specular.r = static_cast<BYTE>(color.r * 255.0F);
                    material.specular.g = static_cast<BYTE>(color.g * 255.0F);
                    material.specular.b = static_cast<BYTE>(color.b * 255.0F);
                    material.specular.a = 255;
                }

                // �L�[���[�h��"map_Kd"��
                if (!strcmp(key, "map_Kd"))
                {
                    // �e�N�X�`���̓ǂݍ���
                    char name[MAX_PATH];
                    fscanf(matfp, "%s ", name);
                    material.texture = LoadGraph(name);
                }
            }

            // �t�@�C�������
            fclose(matfp);
        }

        // �L�[���[�h��'v'��
        if (!strcmp(key, "v"))
        {
            // ���_�\���̂��쐬
            VECTOR pos = { 0 };

            // ���W���i�[
            fscanf(fp, "%f %f %f", &pos.x, &pos.y, &pos.z);

            // �f�[�^��ǉ�
            position.push_back(pos);
        }

        // �L�[���[�h��"vt"��
        if (!strcmp(key, "vt"))
        {
            // �e�N�X�`�����W���i�[
            VECTOR2 vt;
            fscanf(fp, "%f %f", &vt.x, &vt.y);
            uv.push_back(vt);
        }

        // �L�[���[�h��"vn"��
        if (!strcmp(key, "vn"))
        {
            // �@���x�N�g�����i�[
            VECTOR norm;
            fscanf(fp, "%f %f %f", &norm.x, &norm.y, &norm.z);
            normal.push_back(norm);
        }
    }

    // �O�̂��߃t�@�C���|�C���^��擪�ɖ߂�
    fseek(fp, 0L, SEEK_SET);
    std::vector<unsigned short> index;
    std::vector<VERTEX3D> vertex;

    // ���炩���߃��������m�ۂ��Ă���
    // UV�C���f�b�N�X�������_���W�̃C���f�b�N�X����葽���ꍇ��UV�C���f�b�N�X����D�悷��
    if (uv.size() > position.size()) vertex.resize(uv.size());
    else vertex.resize(position.size());

    while (!feof(fp))
    {
        // �L�[���[�h�ǂݍ���
        char key[MAX_PATH] = "";
        fscanf(fp, "%s ", key);

        // �L�[���[�h��"f"��
        if (!strcmp(key, "f"))
        {
            // UV�C���f�b�N�X���̕���������
            if (uv.size() > position.size())
            {
                // ���_���W�A�e�N�X�`�����W�A�@���x�N�g���̃C���f�b�N�X���擾
                int v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3;
                fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);

                // �C���f�b�N�X���i�[
                index.push_back(vt1 - 1);
                index.push_back(vt2 - 1);
                index.push_back(vt3 - 1);


                // VERTEX3D�\���̂Ƀf�[�^���i�[����
                vertex.at(vt1 - 1).pos.x = position.at(v1 - 1).x;
                vertex.at(vt1 - 1).pos.y = position.at(v1 - 1).y;
                vertex.at(vt1 - 1).pos.z = -position.at(v1 - 1).z;

                vertex.at(vt2 - 1).pos.y = position.at(v2 - 1).x;
                vertex.at(vt2 - 1).pos.y = position.at(v2 - 1).y;
                vertex.at(vt2 - 1).pos.z = -position.at(v2 - 1).z;

                vertex.at(vt3 - 1).pos.x = position.at(v3 - 1).x;
                vertex.at(vt3 - 1).pos.y = position.at(v3 - 1).y;
                vertex.at(vt3 - 1).pos.z = -position.at(v3 - 1).z;

                vertex.at(vt1 - 1).norm.x = normal.at(vn1 - 1).x;
                vertex.at(vt1 - 1).norm.y = normal.at(vn1 - 1).y;
                vertex.at(vt1 - 1).norm.z = -normal.at(vn1 - 1).z;

                vertex.at(vt2 - 1).norm.x = normal.at(vn2 - 1).x;
                vertex.at(vt2 - 1).norm.y = normal.at(vn2 - 1).y;
                vertex.at(vt2 - 1).norm.z = -normal.at(vn2 - 1).z;

                vertex.at(vt3 - 1).norm.x = normal.at(vn3 - 1).x;
                vertex.at(vt3 - 1).norm.y = normal.at(vn3 - 1).y;
                vertex.at(vt3 - 1).norm.z = -normal.at(vn3 - 1).z;

                vertex.at(vt1 - 1).u = uv.at(vt1 - 1).x;
                vertex.at(vt1 - 1).v = 1.0f - uv.at(vt1 - 1).y;
                vertex.at(vt2 - 1).u = uv.at(vt2 - 1).x;
                vertex.at(vt2 - 1).v = 1.0f - uv.at(vt2 - 1).y;
                vertex.at(vt3 - 1).u = uv.at(vt3 - 1).x;
                vertex.at(vt3 - 1).v = 1.0f - uv.at(vt3 - 1).y;

                // �}�e���A���J���[��ݒ�
                vertex.at(vt1 - 1).dif = material.diffuse;
                vertex.at(vt2 - 1).dif = material.diffuse;
                vertex.at(vt3 - 1).dif = material.diffuse;
                vertex.at(vt1 - 1).spc = material.specular;
                vertex.at(vt2 - 1).spc = material.specular;
                vertex.at(vt3 - 1).spc = material.specular;
            }
            else
            {
                // ���_���W�A�e�N�X�`�����W�A�@���x�N�g���̃C���f�b�N�X���擾
                int v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3;
                fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);

                // �C���f�b�N�X���i�[
                index.push_back(v1 - 1);
                index.push_back(v2 - 1);
                index.push_back(v3 - 1);

                // VERTEX3D�\���̂Ƀf�[�^���i�[����
                vertex.at(vt1 - 1).pos.x = position.at(v1 - 1).x;
                vertex.at(vt1 - 1).pos.y = position.at(v1 - 1).y;
                vertex.at(vt1 - 1).pos.z = -position.at(v1 - 1).z;

                vertex.at(vt2 - 1).pos.y = position.at(v2 - 1).x;
                vertex.at(vt2 - 1).pos.y = position.at(v2 - 1).y;
                vertex.at(vt2 - 1).pos.z = -position.at(v2 - 1).z;

                vertex.at(vt3 - 1).pos.x = position.at(v3 - 1).x;
                vertex.at(vt3 - 1).pos.y = position.at(v3 - 1).y;
                vertex.at(vt3 - 1).pos.z = -position.at(v3 - 1).z;

                vertex.at(vt1 - 1).norm.x = normal.at(vn1 - 1).x;
                vertex.at(vt1 - 1).norm.y = normal.at(vn1 - 1).y;
                vertex.at(vt1 - 1).norm.z = -normal.at(vn1 - 1).z;

                vertex.at(vt2 - 1).norm.x = normal.at(vn2 - 1).x;
                vertex.at(vt2 - 1).norm.y = normal.at(vn2 - 1).y;
                vertex.at(vt2 - 1).norm.z = -normal.at(vn2 - 1).z;

                vertex.at(vt3 - 1).norm.x = normal.at(vn3 - 1).x;
                vertex.at(vt3 - 1).norm.y = normal.at(vn3 - 1).y;
                vertex.at(vt3 - 1).norm.z = -normal.at(vn3 - 1).z;

                vertex.at(v1 - 1).u = uv.at(vt1 - 1).x;
                vertex.at(v1 - 1).v = 1.0f - uv.at(vt1 - 1).y;
                vertex.at(v2 - 1).u = uv.at(vt2 - 1).x;
                vertex.at(v2 - 1).v = 1.0f - uv.at(vt2 - 1).y;
                vertex.at(v3 - 1).u = uv.at(vt3 - 1).x;
                vertex.at(v3 - 1).v = 1.0f - uv.at(vt3 - 1).y;

                // �}�e���A���J���[��ݒ�
                vertex.at(v1 - 1).dif = material.diffuse;
                vertex.at(v2 - 1).dif = material.diffuse;
                vertex.at(v3 - 1).dif = material.diffuse;
                vertex.at(v1 - 1).spc = material.specular;
                vertex.at(v2 - 1).spc = material.specular;
                vertex.at(v3 - 1).spc = material.specular;
            }

        }
    }

    // �t�@�C�������
    fclose(fp);

    float rotate = 0.0F;

    // ���C�����[�v
    while (ProcessMessage() != -1 && !CheckHitKey(KEY_INPUT_ESCAPE))
    {
        // ��]�s��̍쐬
        rotate++;
        MATRIX world = MGetRotX(rotate * DX_PI_F / 180.0F);
        world = MMult(world, MGetRotY(rotate * DX_PI_F / 180.0F));
        world = MMult(world, MGetRotZ(rotate * DX_PI_F / 180.0F));

        // ��ʏ�����
        ClearDrawScreen();

        // �쐬�������[���h�s���K�p
        //SetTransformToWorld(&world);

        // obj�`��
        DrawPolygonIndexed3D(
            vertex.data(), vertex.size(),
            index.data(), index.size() / 3, material.texture, false);

        // ����ʂɕ`�悵�����e��\�ɕ\������
        ScreenFlip();
    }

    // DX���C�u�����̔j��
    DxLib_End();

    return 0;
}