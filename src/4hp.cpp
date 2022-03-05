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

	/////////////////////////////////////////
	// save and load filepath to JSON file //
	/////////////////////////////////////////

	std::string svgSavePath; // storage for SVG image file path
	bool pathChange = false; // flag to indicate path has changed

	json_t *dataToJson() override 
	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "sPATH", json_string(svgSavePath.c_str()));	
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *savedPATHJ = json_object_get(rootJ, "sPATH");

		if (savedPATHJ) 
		{
			svgSavePath = json_string_value(savedPATHJ);
			pathChange = true;
		}
	}
};

struct customPanel : SvgWidget
{
	_4hp *module; // pointer to module
	std::string svgPath = asset::plugin(pluginInstance, "res/default.svg"); // default image

	customPanel()
	{
		setSvg(Svg::load(svgPath));
	}

	void onPathDrop(const PathDropEvent &e) override
	{
		std::string filePath = e.paths.front(); // get first path in array, all other paths are ignored

		if(filePath.substr(filePath.find_last_of(".") + 1) == "svg") // check for SVG extension
		{
			svgPath = filePath; // store current path

			if (module) // check for module
			{
				module->svgSavePath = svgPath;
				module->pathChange = true;
			}
		}

		else
		{
			DEBUG("Invalid file extension: try *.svg\n");
		}
	}

	void draw(const DrawArgs& args) override
	{
		if (module)
		{
			if (module->pathChange) 
			{
				setSvg(Svg::load(asset::plugin(pluginInstance, module->svgSavePath))); // set new file path
				
				module->pathChange = false;
			}
		}

		SvgWidget::draw(args); // continue to run the rest of the default draw function
	}
};

struct _4hpWidget : ModuleWidget
{
	_4hpWidget(_4hp *module)
	{
		setModule(module);

		// manually create panel and set hidden to hide grey border
		SvgPanel *borderlessPanel = new SvgPanel();
		borderlessPanel->setBackground(Svg::load(asset::plugin(pluginInstance, "res/4hp.svg")));
		borderlessPanel->hide();
		setPanel(borderlessPanel);

		// maunally add widget to allow for pointer reference
		customPanel *CP = new customPanel();
		CP->module = module;
		CP->box.pos = Vec(0, 0);
		CP->box.size = Vec(RACK_GRID_WIDTH * 4, RACK_GRID_HEIGHT);
		addChild(CP);

		if (module)
		{
			if (!module->pathChange) 
			{
				module->svgSavePath = CP->svgPath;
				module->pathChange = true;
			}

			DEBUG("%s\n", module->svgSavePath.c_str());
		}
	}

	void drawLayer(const DrawArgs& args, int layer) override // override drawing module dropshadow
	{
		// do nothing
	}
};

Model *model_4hp = createModel<_4hp, _4hpWidget>("4hp");