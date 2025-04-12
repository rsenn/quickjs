let context = new AudioContext();

let d = context.destination;

let o = new OscillatorNode(context, { type: 'sawtooth', frequency: 1000, channelCount: 2 });

o.connect(d);
o.start();
setTimeout(() => o.stop(), 1000);
