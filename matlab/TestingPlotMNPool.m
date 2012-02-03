%Add the units
%Control size

load ('Z:\Documents\Experiments\R01Pre\NewSpindle\0201StretchReflex1_3VSafety\OutExp020112G\OutExp020112G.mat')
leng=length(Trial.Labview.ExportVar(21).Data);
sampling_rate = 100; % in Hz
range_init=round(leng*0.736):round(leng*0.775);

% time = [0:length(range_init)-1] * (1/sampling_rate);
time=Trial.Labview.Time(range_init);
time = time - time(1);

triallen = length(range_init);

f1=figure(1);
scrsz = get(0,'ScreenSize')
set(f1,'name','One Opposing Muscle','numbertitle','off','Position',[0 0 scrsz(3)/2.2 scrsz(4)])
clf

subplot(411)
plot(time, Trial.Labview.ExportVar(23).Data(range_init))
title('Normalized Joint Angle');
xlim([time(1) time(end)]);
ylim([1.05 1.2]);
ylabel('A.U.')


subplot(412)
plot(time, Trial.Labview.ExportVar(22).Data(range_init))
title('Agonist Group Ia Afferent Firing Rate');
xlim([time(1) time(end)]);
ylim([50,90]);
ylabel('Hz')

% subplot(513)
% plot(time, Trial.Labview.ExportVar(21).Data(range_init))
% title('Agonist Group II Afferent Firing Rate');
% xlim([time(1) time(end)]);
% ylim([90,115]);
% ylabel('Hz')


subplot(414)
plot(time, Trial.Labview.Motor(1).Voltage(range_init))
title('Agonist Tension');
xlim([time(1) time(end)]);
ylim([0.3,1.2]);
ylabel('A.U.')
xlabel('Time(s)');


spikes = Trial.Labview.ExportVar(2).Data(range_init);
for i=1:9
	spikes = [spikes Trial.Labview.ExportVar(2+i*2).Data(range_init)];
end

% figure(2);
% subplot(311)
% plot(Trial.Labview.ExportVar(1).Data(range_init))
% subplot(312)
% plot(Trial.Labview.ExportVar(2).Data(range_init))
% subplot(313)
% plot(Trial.Labview.ExportVar(3).Data(range_init))


spiketimes = find(spikes > 0);

%PLTRSTPTHSMPL(RSTPTHSMPL(spiketimes, spikes,0,1000),'test',1,1,'test2')
subplot(413)
rasterplot(spiketimes,10,length(range_init))
title('Agonist MotoNeuron Raster');

% print(f1,'-dpdf', 'OneOpposingMuscle');

% 
% 
% %28400
% leng=length(Trial.Labview.ExportVar(21).Data);
% range_init=round(leng*0.83):round(leng*0.855);
% %leng
% figure(2);
% %set(f1,'name','Triceps Figures','numbertitle','off')
% clf;
% triallen = length(range_init);
% 
% % subplot(511)
% % plot(Trial.Labview.ExportVar(23+23).Data(range_init))
% % title('Muscle Length');
% % xlim([1,triallen]);
% % ylim([0.9,1.1]);
% % 
% % subplot(512)
% % plot(Trial.Labview.ExportVar(21+23).Data(range_init))
% % title('Dynamic Firing Rate');
% % xlim([1,triallen]);
% % ylim([20,140]);
% % 
% % subplot(513)
% % plot(Trial.Labview.ExportVar(22+23).Data(range_init))
% % title('Static Firing Rate');
% % xlim([1,triallen]);
% % ylim([30,80]);
% % 
% % subplot(514)
% % plot(Trial.Labview.Motor(2).Voltage(range_init))
% % title('Biceps Tension');
% % xlim([1,triallen]);
% 
% 
% 
% 
% 
% 
% % figure(1);
% subplot(512)
% plot(Trial.Labview.ExportVar(21+23).Data(range_init))
% title('Dynamic Firing Rate');
% xlim([1,triallen]);
% ylim([20,140]);
% subplot(511)
% plot(Trial.Labview.ExportVar(23+23).Data(range_init))
% title('Muscle Length');
% xlim([1,triallen]);
% ylim([0.9,1.1]);
% subplot(513)
% plot(Trial.Labview.ExportVar(22+23).Data(range_init))
% title('Static Firing Rate');
% xlim([1,triallen]);
% ylim([30,80]);
% 
% subplot(514)
% plot(Trial.Labview.Motor(2).Voltage(range_init))
% title('Triceps Tension');
% xlim([1,triallen]);
% 
% 
% spikes = Trial.Labview.ExportVar(2+23).Data(range_init);
% for i=1:9
% 	spikes = [spikes Trial.Labview.ExportVar(23+2+i*2).Data(range_init)];
% end
% 
% % figure(2);
% % subplot(311)
% % plot(Trial.Labview.ExportVar(1).Data(range_init))
% % subplot(312)
% % plot(Trial.Labview.ExportVar(2).Data(range_init))
% % subplot(313)
% % plot(Trial.Labview.ExportVar(3).Data(range_init))
% 
% 
% spiketimes = find(spikes > 0);
% 
% %PLTRSTPTHSMPL(RSTPTHSMPL(spiketimes, spikes,0,1000),'test',1,1,'test2')
% subplot(515)
% rasterplot(spiketimes,10,length(range_init))
% title('MotoNeuron Raster');
% 
% last