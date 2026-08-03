#include <backward.hpp>
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include <ktx.h>
#include <set>
#include <argparse/argparse.hpp>
#include <filesystem>

template <typename T>
struct Vec2 {
    public:
        using value_type = T;

    private:
        value_type m_x, m_y;

    public:
        Vec2() : m_x(0), m_y(0) {
        }

        Vec2(const value_type& x, const value_type& y) : m_x(x), m_y(y) {
        }

        explicit Vec2(const value_type& value) : m_x(value), m_y(value) {
        }

        explicit Vec2(const std::initializer_list<value_type>& l) : m_x(l[0]), m_y(l[1]) {
        }

        Vec2 operator+(const Vec2& rhs) const {
            return Vec2(m_x + rhs.m_x, m_y + rhs.m_y);
        }

        Vec2 operator+(const value_type& rhs) const {
            return Vec2(m_x + rhs, m_y + rhs);
        }

        Vec2 operator-(const Vec2& rhs) const {
            return Vec2(m_x - rhs.m_x, m_y - rhs.m_y);
        }

        Vec2 operator-(const value_type& rhs) const {
            return Vec2(m_x - rhs, m_y - rhs);
        }

        Vec2 operator/(const Vec2& rhs) const {
            return Vec2(m_x / rhs.m_x, m_y / rhs.m_y);
        }

        Vec2 operator/(const value_type& rhs) const {
            return Vec2(m_x / rhs, m_y / rhs);
        }

        /// GETTERS
        value_type x() const {
            return m_x;
        }

        value_type y() const {
            return m_y;
        }

        /// SWIZZLE
        Vec2 xy() const {
            return this;
        }

        Vec2 yx() {
            return Vec2(m_y, m_x);
        }

        Vec2 xx() const {
            return Vec2(m_x, m_x);
        }

        Vec2 yy() const {
            return Vec2(m_y, m_y);
        }
};

using IVec2 = Vec2<int>;
using U32Vec2 = Vec2<std::uint32_t>;

template <typename T>
inline std::ostream& operator<<(std::ostream& os, Vec2<T> const& v) requires std::is_integral_v<T> {
    os << "Vec2{.x=" << v.x() << ", .y=" << v.y() << "}";
    return os;
}


struct ImageSpecOptions {
    U32Vec2 tile_size;
    U32Vec2 image_size;
    int nchannels;

    static std::set<VkFormat> support_formats;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;


    std::uint32_t ntiles() const {
        const auto tiles = image_size / tile_size;
        const U32Vec2::value_type ntiles = tiles.x() * tiles.y();
        return ntiles;
    }

    U32Vec2 tiled_dimensions() const {
        return image_size / tile_size;
    }
};

std::set<VkFormat> ImageSpecOptions::support_formats{
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM
};

void gather_options(ImageSpecOptions& options, OIIO::ImageSpec const& spec) {
    options.image_size = U32Vec2(spec.width, spec.height);
    options.nchannels = spec.nchannels;
}

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    argparse::ArgumentParser parser("generate_texture_array_atlas");

    parser.add_argument("--tile-width").required().scan<'i', int>().default_value(16).help("tile width").
           metavar("TILE_WIDTH");
    parser.add_argument("--tile-height").required().scan<'i', int>().default_value(16).help("tile height").
           metavar("TILE_HEIGHT");
    auto& format_options = parser.add_argument("-f", "--format").scan<'i', std::uint32_t>().required().
                                  help("image format").metavar("VK_FORMAT");
    for (auto const& fmt : ImageSpecOptions::support_formats) {
        format_options.add_choice(static_cast<std::uint32_t>(fmt));
    }
    parser.add_argument("-o", "--output").required().help("texture atlas output file").metavar("ATLAS_OUTPUT");
    parser.add_argument("tileset").help("file containing the tileset");

    try {
        parser.parse_args(argc, argv);
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }


    int tw = parser.get<int>("tile-width");
    int th = parser.get<int>("tile-height");

    ImageSpecOptions options{
        .tile_size = U32Vec2(tw, th),
    };

    fs::path tileset = parser.get<std::string>("tileset");

    if (!fs::exists(tileset)) {
        std::cerr << "Tileset does not exist: " << tileset << std::endl;
        return 1;
    }

    auto const input = OIIO::ImageInput::open(tileset);
    std::cout << input->spec().format << '\n';
    std::cout << input->spec().nchannels << '\n';

    gather_options(options, input->spec());
    size_t pxbytes = input->spec().pixel_bytes() * options.image_size.x() * options.image_size.y();
    std::vector<unsigned char> pixels(pxbytes);


    std::cout << "Image Size: " << options.image_size << std::endl;
    std::cout << "Tile Size: " << options.tile_size << std::endl;
    std::cout << "Tiles: " << options.ntiles() << std::endl;
    std::uint32_t iformat = parser.get<std::uint32_t>("format");
    VkFormat format = (VkFormat)iformat;
    if (!ImageSpecOptions::support_formats.contains(format)) {
        std::cerr << "Unsupported format" << std::endl;
        return 1;
    }
    options.format = format;

    if (!input->read_image(0, 0, 0, -1, input->spec().format, pixels.data())) {
        std::cerr << "Failed to read image data from image" << std::endl;
        return 1;
    }


    ktxTextureCreateInfo createInfo{};
    ktx_uint32_t level, layer;
    // TODO: Handle ImageSpec.depth > 1: where image is volumetric.
    createInfo.baseDepth = 1;
    createInfo.baseWidth = options.tile_size.x();
    createInfo.baseHeight = options.tile_size.y();
    createInfo.isArray = true;
    // TODO: if creating 1d Texture nDims=1 else nDims=2 if spec.depth > 0 nDims=3
    createInfo.numDimensions = 2;
    // TODO: if cube map must be 6 else 1.
    createInfo.numFaces = 1;
    createInfo.numLayers = options.ntiles();
    createInfo.numLevels = 1;

    createInfo.vkFormat = static_cast<ktx_uint32_t>(options.format);

    ktxTexture2* texture;
    KTX_error_code result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
    if (result != KTX_SUCCESS) {
        input->close();
        std::cerr << "KTX2 Error (Create): " << ktxErrorString(result) << std::endl;
        return 1;
    }
    std::cout << "Data Size: " << ktxTexture(texture)->dataSize << std::endl;
    layer = 0;
    std::span pxls(pixels);
    size_t bytes_per_pixel = input->spec().pixel_bytes();
    size_t image_stride = options.image_size.x() * bytes_per_pixel;
    size_t tile_stride = options.tile_size.x() * bytes_per_pixel;
    size_t tile_size_bytes = tile_stride * options.tile_size.y();
    std::vector<unsigned char> tile(tile_size_bytes);
    tile.clear();
    for (int y = 0; y < options.tiled_dimensions().y(); y++) {
        for (int x = 0; x < options.tiled_dimensions().x(); x++) {
            size_t tile_x = x * options.tile_size.x();
            size_t tile_y = y * options.tile_size.y();

            for (int row = 0; row < options.tile_size.y(); row++) {
                size_t src_offset =
                    ((tile_y + row) * image_stride) +
                    tile_x * bytes_per_pixel;
                auto const& src = pxls.subspan(src_offset, tile_stride);
                tile.insert(tile.end(), src.begin(), src.end());
                // std::memcpy(
                //     tile.data() + row * tile_stride,
                //     src.data(),
                //     tile_stride);
            }



            result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, layer, 0, tile.data(),
                                          tile.size());
            if (result != KTX_SUCCESS) {
                std::cerr << "Ktx2 Error (SetImageFromMemory): " << ktxErrorString(result) << std::endl;
                return 1;
            }
            layer++;
            tile.clear();

            if (layer >= options.ntiles()) {
                std::cerr << "Layer " << layer << " exceeds number of tiles" << std::endl;
                break;
            }
        }
        if (layer >= options.ntiles()) {
            std::cerr << "Layer " << layer << " exceeds number of tiles" << std::endl;
            break;
        }
    }

    input->close();


    ktx_uint8_t* ktx_object;
    ktx_size_t ktx_object_size;
    ktxTexture_WriteToMemory(ktxTexture(texture), &ktx_object, &ktx_object_size);

    fs::path output = parser.get<std::string>("output");

    std::ofstream outfile;
    outfile.open(output, std::ios::binary);
    outfile.write((char*)ktx_object, ktx_object_size);
    if (outfile.fail()) {
        std::cerr << "Failed to write to output file" << std::endl;
        outfile.close();
        free(ktx_object);
        ktxTexture_Destroy(ktxTexture(texture));
        return 1;
    }

    outfile.close();
    free(ktx_object);
    ktxTexture_Destroy(ktxTexture(texture));


    return 0;


    std::uint32_t tile_w = 16, tile_h = 16;
    const auto image_input = OIIO::ImageInput::open("hello.png");
    OIIO::ImageSpec const& spec = image_input->spec();
    // ImageSpecOptions options;
    options.image_size = U32Vec2(spec.width, spec.height);
    options.tile_size = U32Vec2(tile_w, tile_h);
}

backward::SignalHandling sh;
