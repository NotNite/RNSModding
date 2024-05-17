#include <map>
#include <fstream>
#include <Aurie/shared.hpp>
#include <YYToolkit/Shared.hpp>
using namespace Aurie;

typedef __int64 AllocTextureProto();
typedef void* CreateTextureFromFileProto(void* a1, __int64 a2, int* a3, __int64 a4, char a5);

static AllocTextureProto* AllocTextureOriginal = nullptr;
static CreateTextureFromFileProto* CreateTextureFromFileOriginal = nullptr;

static __int64 last_texture;
static std::map<__int64, void*> g_textures;

/*
 * GR_Texture_Create calls AllocTexture and then CreateTextureFromFile immediately after.
 * Since this texture initialization is called at game startup, the ID (at startup) will match the
 * texture ID in the data.win file. CreateTextureFromFile checks the magic of the file it's reading
 * so you can just pass in PNG data to it.
*/

__int64 AllocTextureDetour()
{
    auto result = AllocTextureOriginal();
    last_texture = result - 1; // What?
    return result;
}

void* CreateTextureFromFileDetour(void* a1, __int64 a2, int* a3, __int64 a4, char a5)
{
    auto it = g_textures.find(last_texture);
    if (it != g_textures.end())
    {
        a1 = it->second;
        last_texture = -1;
    }

    return CreateTextureFromFileOriginal(a1, a2, a3, a4, a5);
}

void* resolve_jmp_call(void* address)
{
    auto offset = *reinterpret_cast<int*>(static_cast<byte*>(address) + 1);
    return static_cast<byte*>(address) + 5 + offset;
}

EXPORTED AurieStatus ModulePreinitialize(
    IN AurieModule* Module,
    IN const fs::path& ModulePath
)
{
    auto module = GetModuleHandle(nullptr);
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(module, path, MAX_PATH);

    auto dir = fs::path(path).parent_path();
    auto textures = dir / "mods" / "TextureSwapper";
    // search *.png files in texture
    for (auto& entry : fs::directory_iterator(textures))
    {
        if (entry.path().extension() == ".png")
        {
            auto texture = entry.path().filename().stem().string();
            auto texture_id = std::stoll(texture);

            auto file = entry.path().string();
            std::ifstream file_stream(file, std::ios::binary);
            std::vector<byte> buffer(std::istreambuf_iterator(file_stream), {});

            auto replacement = malloc(buffer.size());
            memcpy(replacement, buffer.data(), buffer.size());
            g_textures[texture_id] = replacement;
        }
    }

    auto alloc_texture = reinterpret_cast<void*>(MmSigscanModule(
        nullptr, UTEXT("\xE8\x00\x00\x00\x00\x48\x63\xE8\x84\xDB"), "x????xxxxx"));
    if (!alloc_texture) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
    alloc_texture = resolve_jmp_call(alloc_texture);

    auto create_texture_from_file = reinterpret_cast<void*>(MmSigscanModule(
        nullptr, UTEXT("\xE8\x00\x00\x00\x00\xF3\x0F\x10\x15\x00\x00\x00\x00\x4C\x8B\xC0"), "x????xxxx????xxx"));
    if (!create_texture_from_file) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
    create_texture_from_file = resolve_jmp_call(create_texture_from_file);

    void* original = nullptr;
    auto status = MmCreateHook(
        Module,
        "TextureSwapper_AllocTexture",
        alloc_texture,
        AllocTextureDetour,
        &original
    );
    if (!AurieSuccess(status)) return status;
    AllocTextureOriginal = reinterpret_cast<AllocTextureProto*>(original);

    status = MmCreateHook(
        Module,
        "TextureSwapper_CreateTextureFromFile",
        create_texture_from_file,
        CreateTextureFromFileDetour,
        &original
    );
    if (!AurieSuccess(status)) return status;
    CreateTextureFromFileOriginal = reinterpret_cast<CreateTextureFromFileProto*>(original);

    return AURIE_SUCCESS;
}
