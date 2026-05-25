import streamlit as st
import pandas as pd
import time
import os
import plotly.graph_objects as go
import altair as alt
import paho.mqtt.publish as publish

st.set_page_config(page_title="Tractor", layout="wide")
st.title(" Dashboard | RPM | Velocidad | Marcha ")

CSV_FILE = "data/datos_tractor.csv"

def cargar_datos():
    if os.path.exists(CSV_FILE):
        try:
            df = pd.read_csv(CSV_FILE)
            return df.tail(50)
        except Exception as e:
            return pd.DataFrame()
    return pd.DataFrame()

def crear_tacometro(valor, titulo, unidad, max_val, redline_start, color_base="#1f77b4"):
    fig = go.Figure(go.Indicator(
        mode = "gauge+number",
        value = valor,
        domain = {'x': [0, 1], 'y': [0, 1]},
        title = {'text': f"<b>{titulo}</b><br><span style='font-size:0.8em;color:gray'>{unidad}</span>", 'font': {'size': 20}},
        number = {'font': {'size': 50, 'color': 'black'}, 'suffix': f" {unidad}"},
        gauge = {
            'axis': {'range': [None, max_val], 'tickwidth': 2, 'tickcolor': "black", 'nticks': 10},
            'bar': {'color': color_base}, # Color de la aguja/barra principal
            'bgcolor': "white",
            'borderwidth': 2,
            'bordercolor': "gray",
            'steps': [
                {'range': [0, redline_start], 'color': 'lightgray'}, # Zona segura
                {'range': [redline_start, max_val], 'color': '#ff4b4b'} # Zona roja (Alerta)
            ],
            'threshold': {
                'line': {'color': "black", 'width': 3},
                'thickness': 0.75,
                'value': valor # Resaltar valor actual
            }
        }
    ))
    
    fig.update_layout(
        paper_bgcolor = "rgba(0,0,0,0)", # Fondo transparente
        plot_bgcolor = "rgba(0,0,0,0)",
        height=320, 
        margin=dict(l=30, r=30, t=50, b=30)
    )
    return fig

# --- PANEL DE CONTROL (MQTT) ---
st.sidebar.title("Control del Tractor")
st.sidebar.markdown("Usa estos controles para sobreescribir el potenciometro fisico.")
control_remoto = st.sidebar.toggle("Habilitar Control Remoto", value=False)
aceleracion = st.sidebar.slider("Acelerador Virtual (%)", 0, 100, 0, disabled=not control_remoto)

# Construir el payload "ESTADO,VALOR"
estado = 1 if control_remoto else 0
payload = f"{estado},{aceleracion}"

# Publicar solo si el estado o el valor cambian
if "ultimo_payload" not in st.session_state:
    st.session_state.ultimo_payload = None

if payload != st.session_state.ultimo_payload:
    try:
        # Asumimos que el broker esta en localhost, igual que en receptor_mqt.py
        publish.single("tractor/control", payload, hostname="localhost")
        st.session_state.ultimo_payload = payload
    except Exception as e:
        st.sidebar.error(f"Error MQTT: {e}")

# --- DASHBOARD PRINCIPAL ---
df = cargar_datos()

if not df.empty:
    ultimo = df.iloc[-1]
    vel_motor = ultimo['Velocidad del motor']
    vel_vehiculo = ultimo['Velocidad del vehiculo']
    marcha_actual = int(ultimo['Marcha'])

    # --- FILA 1: MARCHA GIGANTE CENTRADA ---
    st.markdown("<h4 style='text-align: center; color: gray; margin-bottom: 0px;'>Marcha</h4>", unsafe_allow_html=True)
    st.markdown(f"<h1 style='text-align: center; font-size: 8rem; color: #ff4b4b; margin-top: -30px; font-weight: bold;'>{marcha_actual}</h1>", unsafe_allow_html=True)
    
    st.divider()
    
    # --- FILA 2: TACOMETROS PROFESSIONALES (Plotly) ---
    col_taco1, col_taco2 = st.columns(2)
    
    with col_taco1:
        fig_rpm = crear_tacometro(vel_motor, "Tacometro Motor", "RPM", 5200, 4400, "#1f77b4")
        st.plotly_chart(fig_rpm, use_container_width=True, key="rpm") 
        
    with col_taco2:
        fig_vel = crear_tacometro(vel_vehiculo, "Tacometro Vehiculo", "km/h", 120, 100, "#ff7f0e")
        st.plotly_chart(fig_vel, use_container_width=True, key="vel")
        
    st.divider()
    
    # --- FILA 3: GRAFICAS CON EJES ETIQUETADOS ---
    col_graf1, col_graf2 = st.columns(2)
    
    df_plot = df.reset_index(drop=True).reset_index()
    df_plot.rename(columns={'index': 'Muestra (historico)'}, inplace=True)

    with col_graf1:
        st.subheader("Tendencia del Motor")
        chart_rpm = alt.Chart(df_plot).mark_line(color="#1f77b4").encode(
            x=alt.X('Muestra (historico)', title='Punto de Muestreo (historico)'),
            y=alt.Y('Velocidad del motor', title='Revoluciones por Minuto (RPM)')
        ).properties(title="Evolucion de RPM")
        st.altair_chart(chart_rpm, use_container_width=True)
        
    with col_graf2:
        st.subheader("Tendencia del Vehiculo")
        chart_vel = alt.Chart(df_plot).mark_line(color="#ff7f0e").encode(
            x=alt.X('Muestra (historico)', title='Punto de Muestreo (historico)'),
            y=alt.Y('Velocidad del vehiculo', title='Velocidad Lineal (km/h)')
        ).properties(title="Evolucion de Velocidad")
        st.altair_chart(chart_vel, use_container_width=True)
        
else:
    st.info("Esperando datos de la ESP32 en el archivo CSV...")
    
time.sleep(0.5) # Tasa de refresco
try:
    st.rerun()
except AttributeError:
    # Compatibilidad con versiones antiguas
    st.experimental_rerun()