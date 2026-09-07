"""
reactor_sim.py — Simulador interactivo de control PID de temperatura
Practica 01 - Sistemas de Control (Python) - Version 1.0.0

Modelo: reactor químico simplificado de 1er orden con retardo térmico,
controlado en lazo cerrado por un PID discreto. El calentador se modela
como una fuente de calor proporcional a la salida del controlador (0-100%),
y hay pérdidas de calor proporcionales a la diferencia con la temperatura
ambiente.

Ejecutar:
    python reactor_sim.py

Requiere:
    numpy
    matplotlib
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, TextBox


# ---------------------------------------------------------------------------
# Modelo de planta: reactor térmico de 1er orden
# ---------------------------------------------------------------------------
class ReactorTermico:
    """
    dT/dt = (Q_heater - Q_loss) / C

    Q_heater = (salida_pid / 100) * potencia_max      [W]
    Q_loss   = k_perdida * (T - T_ambiente)            [W]
    C        = capacidad_termica_efectiva              [J/°C]
    """

    def __init__(self, T0=25.0, T_ambiente=22.0, potencia_max=500.0,
                 k_perdida=8.0, capacidad_termica=1200.0):
        self.T = T0
        self.T_ambiente = T_ambiente
        self.potencia_max = potencia_max
        self.k_perdida = k_perdida
        self.C = capacidad_termica

    def step(self, salida_pid_pct, dt):
        salida_pid_pct = np.clip(salida_pid_pct, 0.0, 100.0)
        q_in = (salida_pid_pct / 100.0) * self.potencia_max
        q_loss = self.k_perdida * (self.T - self.T_ambiente)
        dT = (q_in - q_loss) / self.C * dt
        self.T += dT
        return self.T


# ---------------------------------------------------------------------------
# Controlador PID discreto con anti-windup por saturación (clamping)
# ---------------------------------------------------------------------------
class PID:
    def __init__(self, kp, ki, kd, salida_min=0.0, salida_max=100.0):
        self.kp, self.ki, self.kd = kp, ki, kd
        self.salida_min = salida_min
        self.salida_max = salida_max
        self.integral = 0.0
        self.error_previo = 0.0
        self.primer_paso = True

    def reset(self):
        self.integral = 0.0
        self.error_previo = 0.0
        self.primer_paso = True

    def compute(self, setpoint, medicion, dt):
        error = setpoint - medicion
        salida_p = self.kp * error

        integral_tentativa = self.integral + error * dt
        salida_i_tentativa = self.ki * integral_tentativa

        derivada = 0.0 if self.primer_paso else (error - self.error_previo) / dt
        salida_d = self.kd * derivada

        salida_sin_saturar = salida_p + salida_i_tentativa + salida_d
        salida = np.clip(salida_sin_saturar, self.salida_min, self.salida_max)

        # Anti-windup: solo integra si no está saturado, o si integrar
        # ayuda a salir de la saturación.
        if salida == salida_sin_saturar or (salida_sin_saturar > salida) == (error < 0):
            self.integral = integral_tentativa

        self.error_previo = error
        self.primer_paso = False
        return salida


# ---------------------------------------------------------------------------
# Simulación interactiva
# ---------------------------------------------------------------------------
DT = 0.5          # paso de simulación [s]
DURACION = 600    # ventana visible [s]
N_PUNTOS = int(DURACION / DT)

setpoint = 60.0
kp, ki, kd = 8.0, 0.35, 4.0

planta = ReactorTermico()
pid = PID(kp, ki, kd)

t_buffer = np.zeros(N_PUNTOS)
T_buffer = np.full(N_PUNTOS, planta.T)
sp_buffer = np.full(N_PUNTOS, setpoint)
out_buffer = np.zeros(N_PUNTOS)

fig, (ax_temp, ax_out) = plt.subplots(2, 1, figsize=(9, 7), sharex=True)
plt.subplots_adjust(left=0.1, bottom=0.32, right=0.95, top=0.93, hspace=0.15)

line_T, = ax_temp.plot(t_buffer, T_buffer, color="tab:red", label="Temperatura (°C)")
line_SP, = ax_temp.plot(t_buffer, sp_buffer, color="tab:blue", linestyle="--", label="Setpoint")
ax_temp.set_ylabel("Temperatura [°C]")
ax_temp.set_ylim(15, 100)
ax_temp.legend(loc="upper right")
ax_temp.set_title("Simulador de Control PID — Reactor Térmico (v1.0.0)")
ax_temp.grid(True, alpha=0.3)

line_out, = ax_out.plot(t_buffer, out_buffer, color="tab:green", label="Salida PID (%)")
ax_out.set_ylabel("Potencia calentador [%]")
ax_out.set_xlabel("Tiempo [s]")
ax_out.set_ylim(-5, 105)
ax_out.grid(True, alpha=0.3)
ax_out.legend(loc="upper right")

# --- Controles interactivos ---
ax_kp = plt.axes([0.15, 0.20, 0.7, 0.03])
ax_ki = plt.axes([0.15, 0.15, 0.7, 0.03])
ax_kd = plt.axes([0.15, 0.10, 0.7, 0.03])
ax_sp = plt.axes([0.15, 0.03, 0.25, 0.05])

s_kp = Slider(ax_kp, "Kp", 0.0, 30.0, valinit=kp)
s_ki = Slider(ax_ki, "Ki", 0.0, 2.0, valinit=ki)
s_kd = Slider(ax_kd, "Kd", 0.0, 15.0, valinit=kd)
tb_sp = TextBox(ax_sp, "Setpoint °C  ", initial=str(setpoint))


def on_gains_change(_):
    pid.kp = s_kp.val
    pid.ki = s_ki.val
    pid.kd = s_kd.val


def on_setpoint_change(texto):
    global setpoint
    try:
        setpoint = float(texto)
    except ValueError:
        pass


s_kp.on_changed(on_gains_change)
s_ki.on_changed(on_gains_change)
s_kd.on_changed(on_gains_change)
tb_sp.on_submit(on_setpoint_change)

t_actual = 0.0


def actualizar(frame):
    global t_actual
    salida = pid.compute(setpoint, planta.T, DT)
    temperatura = planta.step(salida, DT)
    t_actual += DT

    T_buffer[:-1] = T_buffer[1:]
    T_buffer[-1] = temperatura
    sp_buffer[:-1] = sp_buffer[1:]
    sp_buffer[-1] = setpoint
    out_buffer[:-1] = out_buffer[1:]
    out_buffer[-1] = salida
    t_buffer[:-1] = t_buffer[1:]
    t_buffer[-1] = t_actual

    line_T.set_data(t_buffer, T_buffer)
    line_SP.set_data(t_buffer, sp_buffer)
    line_out.set_data(t_buffer, out_buffer)
    ax_temp.set_xlim(t_buffer[0], t_buffer[-1] if t_buffer[-1] > 0 else DURACION)
    ax_out.set_xlim(t_buffer[0], t_buffer[-1] if t_buffer[-1] > 0 else DURACION)
    return line_T, line_SP, line_out


if __name__ == "__main__":
    import matplotlib.animation as animation
    ani = animation.FuncAnimation(fig, actualizar, interval=int(DT * 1000), blit=False)
    plt.show()
