% Parameters
Vref = 1; % Reference voltage
N_bits = 8; % Number of bits for ADC resolution
f_clock = 1e6; % Clock frequency (1 MHz)
Tclk = 1 / f_clock; % Clock period

% Calculate the number of clock cycles per conversion
N_clk_per_conversion = (2^N_bits) - 1;

% Calculate the maximum conversion time
max_conversion_time = N_clk_per_conversion * Tclk;

% Calculate the maximum conversion frequency
max_conversion_freq = 1 / max_conversion_time;

% Generate analog input signal (sine wave for example)
Fs = 10 * f_clock; % Sampling frequency
t = 0:1/Fs:0.02; % Time vector with duration 0.02 seconds (two periods)

% Generate clock pulses manually
clk = zeros(size(t));
clk_period = 1 / f_clock;
half_period_samples = round(Fs * clk_period / 2);
for i = 1:half_period_samples:length(t)
    clk(i:min(i + half_period_samples - 1, length(t))) = 1;
    clk(i + half_period_samples:min(i + 2 * half_period_samples - 1, length(t))) = 0;
end

% Initialize analog input and digital output
Analog_Input = zeros(size(t));
Digital_Output = zeros(size(t));

% Initialize comparator, AND gate, and counter outputs
Comparator_Output = zeros(size(t));
AND_Gate_Output = zeros(size(t));
Counter_Output = zeros(size(t));

% Display parameter information during generation
fprintf('Parameter Information:\n');
fprintf('Reference voltage (Vref): %.2f V\n', Vref);
fprintf('Number of bits for ADC resolution (N_bits): %d\n', N_bits);
fprintf('Clock frequency (f_clock): %.2f Hz\n', f_clock);
fprintf('Number of clock cycles per conversion: %d\n', N_clk_per_conversion); % Display the number of clock cycles per conversion
fprintf('Maximum conversion time: %.6f seconds\n', max_conversion_time); % Display the maximum conversion time
fprintf('Maximum conversion frequency: %.2f Hz\n', max_conversion_freq); % Display the maximum conversion frequency
fprintf('\nGenerating signals...\n');

% Generate analog input signal and digital output
for i = 1:length(t)
    Analog_Input(i) = sin(2 * pi * 100 * t(i)); % Example input signal
    Digital_Output(i) = Analog_Input(i) > 0; % Digital output based on comparison
end

% Digital-to-Analog Converter (DAC)
digital_Output_DAC = Digital_Output * Vref; % Convert digital output to analog voltage

% Update the comparator to compare Analog_Input with digital_Output_DAC
for i = 1:length(t)
    if Analog_Input(i) > digital_Output_DAC(i)
        Comparator_Output(i) = 1;
    else
        Comparator_Output(i) = 0;
    end
    
    % AND gate output (with clock pulses)
    AND_Gate_Output(i) = Digital_Output(i) & Comparator_Output(i) & clk(i);
end

% Counter (simple example)
counter_value = 0;
for i = 2:length(t)
    if Digital_Output(i) ~= Digital_Output(i - 1) && clk(i) == 1
        counter_value = counter_value + 1;
    end
    Counter_Output(i) = counter_value;
end

% Plotting
figure;
subplot(3, 1, 1);
plot(t, Analog_Input);
xlabel('Time (s)');
ylabel('Analog Input');
title('Analog Input Signal');

subplot(3, 1, 2);
stairs(t, Digital_Output);
ylim([-0.2, 1.2]);
xlabel('Time (s)');
ylabel('Digital Output');
title('Digital Output from ADC');

subplot(3, 1, 3);
plot(t, Counter_Output);
xlabel('Time (s)');
ylabel('Counter Output');
title('Counter Output');
