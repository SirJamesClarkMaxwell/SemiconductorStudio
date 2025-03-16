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
### Adding many plots   
  - <!-- TODO --> Add Groups with different names
  - <!-- TODO --> iterate through characteristics
    - <!-- TODO --> generate names
    - <!-- TODO --> add plot