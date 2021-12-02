// Include
#include <vector>
#include "DxLib.h"

// テクスチャ座標保存用構造体
struct VECTOR2
{
    float x, y;
};

// マテリアル格納用
struct Material
{
    COLOR_U8 diffuse;   // ディフューズカラー
    COLOR_U8 specular;  // スペキュラカラー
    int texture;        // グラフィックハンドル
};

// WinMain
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpcmdLine, _In_ int nCmdShow)
{
    ChangeWindowMode(true);            // ウィンドウモードに設定
    SetBackgroundColor(100, 149, 237); // 背景カラー

    // DXライブラリの初期化
    if (DxLib_Init() == -1)
    {
        // エラー
        return 0;
    }

    // 裏画面に描画する
    SetDrawScreen(DX_SCREEN_BACK);

    // ライト無効
    SetUseLighting(true);

    // Zバッファ有効
    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(true);

    // カメラ座標と注視点の設定
    SetCameraPositionAndTarget_UpVecY(
        VGet(0.0F, 1.0F, -1.0F), VGet(0.0F, 0.0F, 0.0F));

    // カメラのNEARとFARの設定
    SetCameraNearFar(0.5f, 100.0F);

    // ファイルを開く
    FILE* fp = fopen("Chips.obj", "r");

    // NULLチェック
    if (!fp)
    {
        // エラー
        return 0;
    }

    // 頂点座標、テクスチャ座標、法線ベクトル格納先の用意
    std::vector<VECTOR> position;
    std::vector<VECTOR> normal;
    std::vector<VECTOR2> uv;
    Material material;

    while (!feof(fp))
    {
        char key[MAX_PATH] = "";

        // キーワード読み込み
        fscanf(fp, "%s ", key);

        // キーワードは"mtllib"か
        if (!strcmp(key, "mtllib"))
        {
            // マテリアルファイルの読み込み
            fscanf(fp, "%s ", key);
            FILE* matfp = fopen(key, "r");

            while (!feof(matfp))
            {
                // キーワード読み込み
                fscanf(matfp, "%s ", key);

                // キーワードは"Kd"か
                if (!strcmp(key, "Kd"))
                {
                    // ディフューズカラーの読み込み
                    COLOR_F color;
                    fscanf(matfp, "%f %f %f", &color.r, &color.g, &color.b);

                    // 0～255に変換して格納
                    material.diffuse.r = static_cast<BYTE>(color.r * 255.0F);
                    material.diffuse.g = static_cast<BYTE>(color.g * 255.0F);
                    material.diffuse.b = static_cast<BYTE>(color.b * 255.0F);
                    material.diffuse.a = 255;
                }

                // キーワードは"Ks"か
                if (!strcmp(key, "Ks"))
                {
                    // スペキュラカラーの読み込み
                    COLOR_F color;
                    fscanf(matfp, "%f %f %f", &color.r, &color.g, &color.b);
                    material.specular.r = static_cast<BYTE>(color.r * 255.0F);
                    material.specular.g = static_cast<BYTE>(color.g * 255.0F);
                    material.specular.b = static_cast<BYTE>(color.b * 255.0F);
                    material.specular.a = 255;
                }

                // キーワードは"map_Kd"か
                if (!strcmp(key, "map_Kd"))
                {
                    // テクスチャの読み込み
                    char name[MAX_PATH];
                    fscanf(matfp, "%s ", name);
                    material.texture = LoadGraph(name);
                }
            }

            // ファイルを閉じる
            fclose(matfp);
        }

        // キーワードは'v'か
        if (!strcmp(key, "v"))
        {
            // 頂点構造体を作成
            VECTOR pos = { 0 };

            // 座標を格納
            fscanf(fp, "%f %f %f", &pos.x, &pos.y, &pos.z);

            // データを追加
            position.push_back(pos);
        }

        // キーワードは"vt"か
        if (!strcmp(key, "vt"))
        {
            // テクスチャ座標を格納
            VECTOR2 vt;
            fscanf(fp, "%f %f", &vt.x, &vt.y);
            uv.push_back(vt);
        }

        // キーワードは"vn"か
        if (!strcmp(key, "vn"))
        {
            // 法線ベクトルを格納
            VECTOR norm;
            fscanf(fp, "%f %f %f", &norm.x, &norm.y, &norm.z);
            normal.push_back(norm);
        }
    }

    // 念のためファイルポインタを先頭に戻す
    fseek(fp, 0L, SEEK_SET);
    std::vector<unsigned short> index;
    std::vector<VERTEX3D> vertex;

    // あらかじめメモリを確保しておく
    // UVインデックス数が頂点座標のインデックス数より多い場合はUVインデックス数を優先する
    if (uv.size() > position.size()) vertex.resize(uv.size());
    else vertex.resize(position.size());

    while (!feof(fp))
    {
        // キーワード読み込み
        char key[MAX_PATH] = "";
        fscanf(fp, "%s ", key);

        // キーワードは"f"か
        if (!strcmp(key, "f"))
        {
            // UVインデックス数の方が多いか
            if (uv.size() > position.size())
            {
                // 頂点座標、テクスチャ座標、法線ベクトルのインデックスを取得
                int v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3;
                fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);

                // インデックスを格納
                index.push_back(vt1 - 1);
                index.push_back(vt2 - 1);
                index.push_back(vt3 - 1);


                // VERTEX3D構造体にデータを格納する
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

                // マテリアルカラーを設定
                vertex.at(vt1 - 1).dif = material.diffuse;
                vertex.at(vt2 - 1).dif = material.diffuse;
                vertex.at(vt3 - 1).dif = material.diffuse;
                vertex.at(vt1 - 1).spc = material.specular;
                vertex.at(vt2 - 1).spc = material.specular;
                vertex.at(vt3 - 1).spc = material.specular;
            }
            else
            {
                // 頂点座標、テクスチャ座標、法線ベクトルのインデックスを取得
                int v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3;
                fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);

                // インデックスを格納
                index.push_back(v1 - 1);
                index.push_back(v2 - 1);
                index.push_back(v3 - 1);

                // VERTEX3D構造体にデータを格納する
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

                // マテリアルカラーを設定
                vertex.at(v1 - 1).dif = material.diffuse;
                vertex.at(v2 - 1).dif = material.diffuse;
                vertex.at(v3 - 1).dif = material.diffuse;
                vertex.at(v1 - 1).spc = material.specular;
                vertex.at(v2 - 1).spc = material.specular;
                vertex.at(v3 - 1).spc = material.specular;
            }

        }
    }

    // ファイルを閉じる
    fclose(fp);

    float rotate = 0.0F;

    // メインループ
    while (ProcessMessage() != -1 && !CheckHitKey(KEY_INPUT_ESCAPE))
    {
        // 回転行列の作成
        rotate++;
        MATRIX world = MGetRotX(rotate * DX_PI_F / 180.0F);
        world = MMult(world, MGetRotY(rotate * DX_PI_F / 180.0F));
        world = MMult(world, MGetRotZ(rotate * DX_PI_F / 180.0F));

        // 画面初期化
        ClearDrawScreen();

        // 作成したワールド行列を適用
        //SetTransformToWorld(&world);

        // obj描画
        DrawPolygonIndexed3D(
            vertex.data(), vertex.size(),
            index.data(), index.size() / 3, material.texture, false);

        // 裏画面に描画した内容を表に表示する
        ScreenFlip();
    }

    // DXライブラリの破棄
    DxLib_End();

    return 0;
}