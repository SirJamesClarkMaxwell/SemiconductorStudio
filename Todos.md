### Light characteristics
- <!-- TODO--> add characteristic type
- <!-- TODO--> update MoodelIDs
- <!-- TODO--> add estimators
- <!-- TODO--> add fitters
- <!-- TODO--> register (pre)fitters
- <!-- TODO--> add in UI option to change type of characteristic

### Dumping data into the file 
- <!-- TODO--> Check file existence 
- <!-- TODO--> If there are no file: create it  
- <!-- TODO--> If there is a file override the content 
- <!-- TODO--> save file based on the path current directory 

### Dumping MonteCarlo Data 
 - <!-- TODO--> If there are no MonteCarlo directory $\rightarrow$ create it 
 - **NOTE** In for loop for all mc data 
   - <!-- TODO--> Sort Data based on the error 
   - <!-- TODO--> Build name of the file based on the name of characteristic and sigma 
   - <!-- TODO--> Write data in to concrete file 

  
### Fixing tunning sliders
| Enable | Name | Value | Power      | Fix |
|--------|------|-------|-----------|-----|
| ✅     | I0   | 1.5   | $10^{-9}$  | ✅  |
| ✅     | A    | 2     | $10^{0}$   | ✅  |
| ...    | ...  | ...   | ...        |     |
- <!-- TODO--> Iterate through parameters
  - <!-- TODO--> Print one line
    - <!-- TODO--> check enabling option, and fixing option
    - <!-- TODO--> print value slider
    - <!-- TODO--> print power slider
  ```cpp
  						for (auto& [id, value] : act.tunedParameters) 
						{
							float val = value;
							float min = act.fittedParameters[id] / 100.0;
							float max = act.fittedParameters[id] * 100.0;

							ImGui::TableNextColumn();
							std::string cname = "##" + data.paramConfig->parameters[id];
							if (ImGui::Checkbox(cname.c_str(), &act.tempParametersActive[id]))
								if (act.tempParametersActive[id]) data.m_tuneCallback();

							ImGui::SameLine();
							ImGuiSliderFlags flags = ImGuiSliderFlags_Logarithmic & ImGuiSliderFlags_None;

							//and data.paramConfig->parameters[id] != std::string{ "A" }
							//if (data.paramConfig->parameters[id] == std::string{ "I0" } )
							//{
							//	float deciMin{ 1 }, deciMax{ 9 }, powMin{ -20 }, powMax{ -3 }, powValue{std::floor(std::log10(value))}, deciValue{value/std::pow(10,powValue)};
							//	if(ImGui::SliderFloat("I0 decimal", &deciValue, deciMin, deciMax, "%e"))
							//	{
							//		value = deciValue * std::pow(10, powValue);
							//		if (act.tempParametersActive[id]) data.m_tuneCallback();
							//	}
							//	//ImGui::SameLine();
							//	if (ImGui::SliderFloat("I0 pow", &powValue, powMin, powMax, "%e"))
							//	{
							//		value = deciValue * std::pow(10, powValue);
							//		if (act.tempParametersActive[id]) data.m_tuneCallback();
							//	}
							//}
							//else 
							try
							{

							if (ImGui::SliderFloat(data.paramConfig->parameters[id].c_str(), &val, min, max, "%e", flags))
							{
								value = val;
								if (act.tempParametersActive[id]) data.m_tuneCallback();
							}
							}
							catch (const std::exception&e)
							{
								std::cout << e.what()<<std::endl;
							}
							ImGui::SameLine(0.0f, 10.0f);
```


  -5388.19699491, 
  -5388.19652778, 
  -5388.19939656, 
  -5388.20111646, 
  -5388.20108493, 
  -5388.20157217, 
  -5388.20172268, 
  -5388.20216923, 
  -5388.20215916, 
  -5388.20288218, 
  -5388.20475567, 
  -5388.20086475, 
  -5388.20499955, 
  -5388.20508752, 
  -5388.20540663, 
  -5388.20594661, 
  -5388.20663581, 
  -5388.20531353, 
  -5388.20668679, 
  -5388.20769470, 
  -5388.21034772, 
  -5388.21317715, 
  -5388.21321948, 
  -5388.21528727, 
  -5388.22001931, 
  -5388.22291526, 
  -5388.22518116, 
  -5388.23484924, 
  -5388.26544398, 
  -5388.35299062, 
  -5388.49079157, 
  -5388.49497746, 
  -5388.49876961, 
  -5388.49944319, 
  -5388.50132245, 
  -5388.50432877, 
  -5388.50726564, 
  -5388.51085990, 
  -5388.52170425, 
  -5388.54311360, 
  -5388.58484417, 
  -5388.66372520, 
  -5388.80213464, 
  -5388.99327540, 
  -5388.98123334, 
  -5389.05760846, 
  -5389.09286253, 
  -5389.15113112, 
  -5389.15473924, 
  -5389.17803382, 
  -5389.19088241, 
  -5389.20187847, 
  -5389.20573692, 
  -5389.21179992, 
  -5389.21373569, 
  -5389.21654683, 
  -5389.21704931, 
  -5389.21868435, 
  -5389.21876607, 
  -5389.21956773, 
  -5389.21955783, 
  -5389.21993501, 
  -5389.21992534, 
  -5389.22009505, 
  -5389.22009065, 
  -5389.22019029, 
  -5389.22018829, 
  -5389.22024527, 
  -5389.22024338, 
  -5389.22027485 



  -5388.02392273,
  -5388.02350836,
  -5388.02635061,
  -5388.02807811,
  -5388.02804583,
  -5388.02853242,
  -5388.02868572,
  -5388.02913200,
  -5388.02912218,
  -5388.02985134,
  -5388.03171561,
  -5388.02757272,
  -5388.03195079,
  -5388.03203581,
  -5388.03235370,
  -5388.03291032,
  -5388.03358226,
  -5388.03234514,
  -5388.03363757,
  -5388.03463654,
  -5388.03728656,
  -5388.04024984,
  -5388.04037242,
  -5388.04259130,
  -5388.04827794,
  -5388.05520937,
  -5388.05114830,
  -5388.05649362,
  -5388.05740927,
  -5388.06051341,
  -5388.06772965,
  -5388.08543179,
  -5388.12782807,
  -5388.19017211,
  -5388.19145793,
  -5388.19342777,
  -5388.19671366,
  -5388.20488234,
  -5388.21239669,
  -5388.22284232,
  -5388.25381613,