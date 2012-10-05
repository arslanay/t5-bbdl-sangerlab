function CollectionOutput = config_PXI(CollectionInput)
% CollectionInput.HelperFunctions (not used)
% CollectionArgs.PathSep (a string '/' in mac and '' in windows)
%

set(CollectionInput.ProgramStatusIndicator,'string','Collection Method "config_PXI" running... ');
pause(0.1);

saveFile = 'OutExp020912A.txt';
savePath = 'John';



ActiveMotorInd=1+[1 2]';
% ActiveMotorInd = 1+[8];
numActiveMotors = length(ActiveMotorInd);

samplingRate = 100;
bufferSize = 1.0;   % 1 second
numViconMarkers = 0;
MN = 10;
Mus = 2;
ExpSlotInd = 2;
ExtraVars = 4;
numExportVar = Mus*(MN*ExpSlotInd + ExtraVars) ;


configFileName = 'ConfigExp020912A.txt';
configDir = 'John';

auxVarLabels = {'Neuron volt.';'Neuron RecovVar';'Binary Spike';'Spindle FireConst';'Spindle Len';'Spindle Vel';'Ia FireRate';'Muscle Force';'Muscle PrevForce';'LCE';'dx0';'dx1';'dx2';'Spindle FireConst x3';'Spindle Len x4';'Spindle Vel x5';'Ia_2 FireRate';'dx3';'dx4';'dx5';'LenOrigBic';'Neuron volt.';'Neuron RecovVar';'Binary Spike';'Spindle FireConst x0';'Spindle Len x1';'Spindle Vel x2';'Ia FireRate';'Muscle Force';'Muscle PrevForce';'LCE';'dx0';'dx1';'dx2';'Spindle FireConst x3';'Spindle Len x4';'Spindle Vel x5';'Ia_2 FireRate';'dx3';'dx4';'dx5';'LenOrigTri'};
initAuxVar = [-70.0 -14.0 0 0 0.9579 0 0 0 0 0 0 0 0 0.0 0.9576 0 0 0 0 0 0 -70.0 -14.0 0 0 0.9579 0 0 0 0 0 0 0 0 0.0 0.9576 0 0 0 0 0 0 ];
paramLabels = {'BicAlpha','TriAlpha','MN Digital Spike','GammaDyn','MF TimeConst','MF Peak Force','RESET','OutVolScale','Bag1FRtoI','LenScaleBic','LenScaleTri','GammaStatic','Bag2FRtoI'};
userLabels = {  'MN0B Volt','MN0B Rec','MN0B Spike','MN1B Volt','MN1B Rec','MN1B Spike','MN2B Volt','MN2B Rec','MN2B Spike','MN3B Volt','MN3B Rec','MN3B Spike','MN4B Volt','MN4B Rec','MN4B Spike',...
                'MN5B Volt','MN5B Rec','MN5B Spike','MN6B Volt','MN6B Rec','MN6B Spike','MN7B Volt','MN7B Rec','MN7B Spike','MN8B Volt','MN8B Rec','MN8B Spike','MN9B Volt','MN9B Rec','MN9B Spike',...
                'MN0T Volt','MN0T Rec','MN0T Spike','MN1T Volt','MN1T Rec','MN1T Spike','MN2T Volt','MN2T Rec','MN2T Spike','MN3T Volt','MN3T Rec','MN3T Spike','MN4T Volt','MN4T Rec','MN4T Spike',...
                'MN5T Volt','MN5T Rec','MN5T Spike','MN6T Volt','MN6T Rec','MN6T Spike','MN7T Volt','MN7T Rec','MN7T Spike','MN8T Volt','MN8T Rec','MN8T Spike','MN9T Volt','MN9T Rec','MN9T Spike'};
initUser = [-70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 ...
            -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 ...
            -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 ...
            -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 -70.0 -14.0 0 ];


% FFS parameters
% Inputs
% mags =[.5 1 2 3];
% H = (unique(nchoosek([mags mags mags],numActiveMotors),'rows'))';

%mags =[4];
%H = (unique(nchoosek([mags],numActiveMotors),'rows'))';


% H = [.5 1 2 3 4 4 4 4;
%      .5 1 2 3 4 5 6 7;
%      .5 1 2 3 4 4 4 4];
%

%H = zeros(1,20);


%get to 40
H = [0 ; 0 ; 30 ; 80; 0.05; 0.03 ; 0 ; 0.3 ; 0.1 ; 0.001; 0.001 ; 40; 1.2];
%H = [.5 ;...
%     1 ;...
%     3 ;...
%     1 ;...
%     1 ;...
%     3 ;...
%     3 ;...
%     1 ;...
%     3 ;...
%       2.6469    ;...
%    2.3897    ;...
%    1.6342    ;...
%    2.9004    ;...
%    1.0689    ]';

%H = [H,fliplr(H)];
%numInputs = size(H,2)
%
%H2 = zeros(20,numInputs);
%H2(ActiveMotorInd,1:numInputs) = H;
%
%H = reshape(H2,20*numInputs,1);

initParam = H;

cd FtpTemp
fid = fopen(configFileName,'w');

fprintf(fid,'%.4f\n',samplingRate);
fprintf(fid,'%.4f\n',bufferSize);
fprintf(fid,'%d\n',numViconMarkers);
fprintf(fid,'%d\n',numExportVar);
fprintf(fid,'%s\n',['c:\' savePath '\' saveFile]);

for i = 1:length(auxVarLabels)-1
    fprintf(fid,'%s,',auxVarLabels{i});
end
fprintf(fid,'%s',auxVarLabels{length(auxVarLabels)});
fprintf(fid,'\n');
for i = 1:length(initAuxVar)-1
    fprintf(fid,'%.4f,',initAuxVar(i));
end
fprintf(fid,'%.4f',initAuxVar(length(initAuxVar)));
fprintf(fid,'\n');

for i = 1:length(paramLabels)-1
    fprintf(fid,'%s,',paramLabels{i});
end
fprintf(fid,'%s',paramLabels{length(paramLabels)});
fprintf(fid,'\n');
for i = 1:length(initParam)-1
    fprintf(fid,'%.4f,',initParam(i));
end
fprintf(fid,'%.4f',initParam(length(initParam)));
fprintf(fid,'\n');

for i = 1:length(userLabels)-1
    fprintf(fid,'%s,',userLabels{i});
end
fprintf(fid,'%s',userLabels{length(userLabels)});
fprintf(fid,'\n');
for i = 1:length(initUser)-1
    fprintf(fid,'%.4f,',initUser(i));
end
fprintf(fid,'%.4f',initUser(length(initUser)));

fclose(fid);

f = ftp('192.168.10.100');
cd(f,configDir);
mput(f,configFileName);
cd(f,'/');

cd(f,savePath);
a = dir(f,saveFile);
if isempty(a) == 0
    UserInput = questdlg('A previous trial of the same name was found. Are you sure you want to delete it from Brachioplex?');
    switch UserInput
        case 'Yes'
            delete(f,saveFile);
            msgbox('Previous trial deleted');
        case 'No'
            msgbox('Previous trial not deleted. WARNING: running the sequencer now will append to previous trial.');
        case 'Cancel'
    end
end
    
cd ..

set(CollectionInput.ProgramStatusIndicator,'string','End of collection method "config_PXI" ');
pause(1);
set(CollectionInput.ProgramStatusIndicator,'string','Ready.');


CollectionOutput = 1;