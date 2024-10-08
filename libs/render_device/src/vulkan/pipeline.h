#pragma once

struct RenderPipeline;
struct GraphicsState;
namespace vulkan
{
class Pipeline
{
  public:
    Pipeline();
    ~Pipeline();
    Pipeline(const Pipeline&);
    // Pipeline(Pipeline&&)                 = delete;
    //Pipeline& operator=(const Pipeline&) = delete;
    //Pipeline& operator=(Pipeline&&)      = delete;

    static Pipeline Create(VkDevice              device,
                           const RenderPipeline& pipeline,
                           const GraphicsState&  graphicsState);

    bool operator==(const Pipeline&) const noexcept = default;

    void Destroy(VkDevice device);

    VkPipeline       GetPipeline() const;
    VkPipelineLayout GetPipelineLayout() const;

  protected:
    static VkShaderModule CreateShaderModule(VkDevice device, const char* filename);

  private:
    VkPipeline       m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
};
} // namespace vulkan
