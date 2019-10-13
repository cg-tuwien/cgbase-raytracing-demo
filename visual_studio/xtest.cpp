#include <cg_base.hpp>

class xtest_app : public cgb::cg_element
{

public: // v== cgb::cg_element overrides which will be invoked by the framework ==v

	void initialize() override
	{
		// Load a collada scene from file:
		auto loadedscene1 = cgb::model_t::load_from_file("assets/level01c.dae");
		auto loadedscene2 = cgb::model_t::load_from_file("assets/level01cf1.fbx");
		// Get all the different materials from the whole scene:
		auto distinctMaterials = loadedscene2->distinct_material_configs();

	}

	void update() override
	{
		
	}

	void render() override
	{
		

	}

	void finalize() override
	{
		cgb::context().logical_device().waitIdle();
	}

}; // focus_rt_app

//int main() // <== Starting point ==
//{
//	try {
//		// What's the name of our application
//		cgb::settings::gApplicationName = "cg_base::focus_rt";
//		cgb::settings::gQueueSelectionPreference = cgb::device_queue_selection_strategy::prefer_everything_on_single_queue;
//		cgb::settings::gRequiredDeviceExtensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
//		cgb::settings::gRequiredDeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
//
//		// Create a window and open it
//		auto mainWnd = cgb::context().create_window("Focus!");
//		mainWnd->set_resolution({ 1600, 900 });
//		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
//		mainWnd->set_additional_back_buffer_attachments({ cgb::attachment::create_depth(cgb::image_format::default_depth_format()) });
//		mainWnd->open();
//
//		// Create an instance of vertex_buffers_app which, in this case,
//		// contains the entire functionality of our application. 
//		auto element = xtest_app();
//
//		// Create a composition of:
//		//  - a timer
//		//  - an executor
//		//  - a behavior
//		// ...
//		auto hello = cgb::composition<cgb::varying_update_timer, cgb::sequential_executor>({
//				&element
//			});
//
//		// ... and start that composition!
//		hello.start();
//	}
//	catch (std::runtime_error & re)
//	{
//		LOG_ERROR_EM(re.what());
//		//throw re;
//	}
//}
//
//