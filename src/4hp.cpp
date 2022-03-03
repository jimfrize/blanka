#include "plugin.hpp"
	
	struct _4hp : Module
	{
		enum ParamId
		{
			PARAMS_LEN
		};
		enum InputId
		{
			INPUTS_LEN
		};
		enum OutputId
		{
			OUTPUTS_LEN
		};
		enum LightId
		{
			LIGHTS_LEN
		};
	
		_4hp()
		{
			config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		}
	
		std::string jsonSavePath; // storage for SVG image file path
		bool pathDirty = false; // flag to indicate path has been changed
	
		// save and load filepath to JSON file
	
		json_t *dataToJson() override 
		{
			json_t *rootJ = json_object();
			json_object_set_new(rootJ, "sPATH", json_string(jsonSavePath.c_str()));	
			return rootJ;
		}
	
		void dataFromJson(json_t *rootJ) override
		{
			json_t *savedPATHJ = json_object_get(rootJ, "sPATH");
			if (savedPATHJ) {
				jsonSavePath = json_string_value(savedPATHJ);
				pathDirty = true;
			}
		}
	};
	
	struct customPanel : SvgWidget
	{
		_4hp *module;
		customPanel()
		{
			// Does nothing because PATH is empty on construction
			//	setSvg(Svg::load(asset::plugin(pluginInstance, PATH))); // set SVG file on creation of customPanel
		}
	
		void draw(const DrawArgs& args) override
		{
			if (module) {
				if (module->pathDirty) {
					module->pathDirty = false;
					setSvg(Svg::load(asset::plugin(pluginInstance, module->jsonSavePath))); // set new file path
				}
			}
	
			SvgWidget::draw(args); // continue to run the rest of the default draw function
		}
	};
	
	struct dnd : TransparentWidget
	{
		_4hp *module;
		std::string svgPath = asset::plugin(pluginInstance, "res/default.svg"); // path with SVG extension
	
		void onPathDrop(const PathDropEvent &e) override
		{
			std::string filePath = e.paths.front(); // get first path in array, all other paths are ignored
	
			if(filePath.substr(filePath.find_last_of(".") + 1) == "svg") // check for SVG extension
			{
				svgPath = filePath; // store path
				if (module) {
					module->jsonSavePath = svgPath;
					module->pathDirty = true;
				}
			}
	
			else
			{
				DEBUG("Invalid file extension: try *.svg\n");
			}
		}
	};
	
	struct _4hpWidget : ModuleWidget
	{
		_4hpWidget(_4hp *module)
		{
			setModule(module);
			setPanel(createPanel(asset::plugin(pluginInstance, "res/4hp.svg")));
	
			// maunally add widgets to allow for pointer references
	
			dnd *DND = new dnd();
			DND->module = module;
			DND->box.pos = Vec(0, 0);
			DND->box.size = Vec(RACK_GRID_WIDTH * 4, RACK_GRID_HEIGHT);
			addChild(DND);
	
			customPanel *CP = new customPanel();
			CP->module = module;
			CP->box.pos = Vec(0, 0);
			CP->box.size = Vec(RACK_GRID_WIDTH * 4, RACK_GRID_HEIGHT);
			addChild(CP);
	
			if (module)
			{
				if (!module->pathDirty) {
					module->jsonSavePath = DND->svgPath;
					module->pathDirty = true;
				}
				DEBUG("%s\n", module->jsonSavePath.c_str());
			}
		}
	};
	
	Model* model_4hp = createModel<_4hp, _4hpWidget>("4hp");