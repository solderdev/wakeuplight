import numpy as np
import matplotlib.pyplot as plt

duty_max = 80
duty_min = 0.001
fade_minutes = 40

time_axis = np.arange(0, fade_minutes + 1)
duty_lin = time_axis * duty_max / fade_minutes
duty_2 = time_axis ** 2 * duty_max / fade_minutes ** 2
duty_3 = time_axis ** 3 * duty_max / fade_minutes ** 3


def calc_duty_sig(sig_width, sig_offset):  # 5, 0.8 is nice!
    # sig_width: smaller --> smoother
    # sig_offset (0 to x): 1 = middle ; higher = earlier ; lower (>0) = later
    duty_sig = 1 / (sig_offset + np.exp(-sig_width * (time_axis - fade_minutes / 2) / fade_minutes * 2))
    duty_sig *= (duty_max - duty_min) / (duty_sig[-1] - duty_sig[0])
    duty_sig -= duty_sig[0] - duty_min
    print(f'duty_sig: min: {duty_sig[0]:.4f}  max: {duty_sig[-1]:.4f}')
    return duty_sig


duty_sine = (duty_max + np.cos(np.pi + time_axis / fade_minutes * np.pi) * (duty_max - duty_min)) / 2

print(f'time: min: {time_axis[0]:.4f}  max: {time_axis[-1]:.4f}')
print(f'duty_lin: min: {duty_lin[0]:.4f}  max: {duty_lin[-1]:.4f}')
print(f'duty_2: min: {duty_2[0]:.4f}  max: {duty_2[-1]:.4f}')
print(f'duty_3: min: {duty_3[0]:.4f}  max: {duty_3[-1]:.4f}')
print(f'duty_sine: min: {duty_sine[0]:.4f}  max: {duty_sine[-1]:.4f}')

fig = plt.figure()
plt.plot(time_axis, duty_lin, label='linear')
plt.plot(time_axis, duty_2, label='^2')
plt.plot(time_axis, duty_3, label='^3')
# for i in range(2, 8):
#     plt.plot(time_axis, calc_duty_sig(i, 0.3), label=f'duty_sig: w={i}')
for i in [0.3, 0.5, 0.8, 1.2, 2]:
    plt.plot(time_axis, calc_duty_sig(5, i), label=f'duty_sig: w={5} o={i}')
plt.plot(time_axis, duty_sine, label='duty_sine')

plt.grid()
plt.legend()
plt.show()
