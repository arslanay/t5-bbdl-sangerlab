clf
range_init=1000:length(Trial.Labview.ExportVar(1).Data);
figure(1);

subplot(311)
plot(Trial.Labview.ExportVar(2).Data(range_init))
subplot(312)
plot(Trial.Labview.ExportVar(4).Data(range_init))
subplot(313)
plot(Trial.Labview.ExportVar(6).Data(range_init))

spikes = Trial.Labview.ExportVar(2).Data(range_init);
spikes = [spikes Trial.Labview.ExportVar(4).Data(range_init)];

spikes = [spikes Trial.Labview.ExportVar(6).Data(range_init)];

spiketimes = find(spikes > 0);

%PLTRSTPTHSMPL(RSTPTHSMPL(spiketimes, spikes,0,1000),'test',1,1,'test2')
rasterplot(spiketimes,3,length(range_init))
