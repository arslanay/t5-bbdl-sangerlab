clf
range_init=10000:15000	%length(Trial.Labview.ExportVar(1).Data);
figure(1);

% subplot(1011)
% plot(Trial.Labview.ExportVar(2).Data(range_init))
% subplot(312)
% plot(Trial.Labview.ExportVar(4).Data(range_init))
% subplot(313)
% plot(Trial.Labview.ExportVar(6).Data(range_init))

spikes = Trial.Labview.ExportVar(2).Data(range_init);
for i=1:9
	spikes = [spikes Trial.Labview.ExportVar(2+i*2).Data(range_init)];
end


spiketimes = find(spikes > 0);

%PLTRSTPTHSMPL(RSTPTHSMPL(spiketimes, spikes,0,1000),'test',1,1,'test2')
rasterplot(spiketimes,10,length(range_init))
