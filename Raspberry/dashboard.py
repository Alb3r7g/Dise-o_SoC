import streamlit as st
import pandas as pd
import time
import os
import plotly.graph_objects as go
import altair as alt
import paho.mqtt.publish as publish

# Aqui establecemos la configuracion inicial de la pagina del dashboard
st.set_page_config(page_title="John Deere - Telemetria", layout="wide")

# Aqui inyectamos estilos CSS personalizados para lograr la estetica visual de John Deere
st.markdown(
    """
    <style>
    /* Fondo principal claro */
    .stApp {
        background-color: #f4f5f0 !important;
    }
    
    /* Fondo de la Sidebar */
    section[data-testid="stSidebar"] {
        background-color: #367C2B !important; /* Verde */
        border-right: 4px solid #FFDE00 !important; /* Linea amarilla */
    }
    
    /* Configuracion de textos en la Sidebar */
    section[data-testid="stSidebar"] h1, 
    section[data-testid="stSidebar"] h2, 
    section[data-testid="stSidebar"] h3, 
    section[data-testid="stSidebar"] p, 
    section[data-testid="stSidebar"] span {
        color: #FFFFFF !important;
    }
    
    /* Titulos principales en verde */
    h1, h2, h3 {
        color: #367C2B !important; 
    }
    
    /* Slider de Streamlit color amarillo */
    .stSlider > div > div > div > div {
        background-color: #FFDE00 !important;
    }
    </style>
    """,
    unsafe_allow_html=True
)

# Aqui estructuramos el encabezado principal y cargamos el logotipo oficial
col_title, col_logo = st.columns([4, 1])
with col_title:
    st.title(" Dashboard | RPM | Velocidad | Marcha")
with col_logo:
    # Cargamos la imagen del logotipo
    st.image("https://upload.wikimedia.org/wikipedia/en/thumb/0/02/John_Deere_Logo_%E2%80%93_Flat_2_Color.svg/1280px-John_Deere_Logo_%E2%80%93_Flat_2_Color.svg.png", use_container_width=True)

# Aqui configuramos el cliente de InfluxDB y la carga de datos
from influxdb_client import InfluxDBClient

INFLUX_TOKEN = "rfxAIyr4V8zIpavmvmWmZb7iTb0V57Idw6z-yck9jPy59pjN3i3p3i8QhukYLTtiIo5JYSdQwYrdMSXUoQeY-g=="
INFLUX_ORG = "FacultadIng" 
INFLUX_URL = "http://localhost:8086"
INFLUX_BUCKET = "telemetria_tractor"

def cargar_datos():
    try:
        client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
        query_api = client.query_api()
        
        # Aqui consultamos los datos historicos recientes aplicando el pivotado por columnas
        query = f'''
        from(bucket: "{INFLUX_BUCKET}")
          |> range(start: -5m)
          |> filter(fn: (r) => r["_measurement"] == "telemetria_remota_tractor")
          |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
          |> tail(n: 50)
        '''
        
        result = query_api.query_data_frame(query)
        client.close()
        
        if isinstance(result, list):
            if len(result) > 0:
                df = pd.concat(result)
            else:
                return pd.DataFrame()
        else:
            df = result
            
        if not df.empty:
            # Aqui ordenamos la informacion cronologicamente y normalizamos los nombres de las columnas
            df = df.sort_values('_time')
            if 'rpm' in df.columns:
                df = df.rename(columns={
                    'rpm': 'Velocidad del motor',
                    'velocidad': 'Velocidad del vehiculo',
                    'marcha': 'Marcha'
                })
            return df
        return pd.DataFrame()
    except Exception as e:
        st.error(f"Error consultando InfluxDB: {e}")
        return pd.DataFrame()

def crear_tacometro(valor, titulo, unidad, max_val, redline_start, color_base="#367C2B"):
    fig = go.Figure(go.Indicator(
        mode = "gauge+number",
        value = valor,
        domain = {'x': [0, 1], 'y': [0, 1]},
        title = {'text': f"<b>{titulo}</b><br><span style='font-size:0.8em;color:gray'>{unidad}</span>", 'font': {'size': 20}},
        number = {'font': {'size': 50, 'color': 'black'}, 'suffix': f" {unidad}"},
        gauge = {
            'axis': {'range': [None, max_val], 'tickwidth': 2, 'tickcolor': "black", 'nticks': 10},
            'bar': {'color': color_base}, # Aplicamos el color corporativo para la barra indicadora
            'bgcolor': "white",
            'borderwidth': 2,
            'bordercolor': "gray",
            'steps': [
                {'range': [0, redline_start], 'color': 'lightgray'}, 
                {'range': [redline_start, max_val], 'color': '#FFDE00'} # Definimos el umbral de limite en color amarillo
            ],
            'threshold': {
                'line': {'color': "black", 'width': 3},
                'thickness': 0.75,
                'value': valor
            }
        }
    ))
    
    fig.update_layout(
        paper_bgcolor = "rgba(0,0,0,0)",
        plot_bgcolor = "rgba(0,0,0,0)",
        height=320, 
        margin=dict(l=30, r=30, t=50, b=30)
    )
    return fig

def obtener_seguridad():
    try:
        client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
        query_api = client.query_api()
        query = f'''
        from(bucket: "{INFLUX_BUCKET}")
          |> range(start: -1m)
          |> filter(fn: (r) => r["_measurement"] == "seguridad_entorno")
          |> filter(fn: (r) => r["_field"] == "estado_deteccion")
          |> last()
        '''
        result = query_api.query_data_frame(query)
        client.close()
        
        if isinstance(result, list):
            if len(result) > 0:
                result = result[0]
            else:
                return 0
                
        if not result.empty and '_value' in result.columns:
            return int(result['_value'].iloc[0])
    except Exception as e:
        pass
    return 0

# Aqui construimos la barra lateral de controles interactivos
st.sidebar.title("Control del Tractor")
control_remoto = st.sidebar.toggle("Habilitar Control Remoto", value=False)
aceleracion = st.sidebar.slider("Acelerador Virtual (%)", 0, 100, 0, disabled=not control_remoto)
freno_remoto = st.sidebar.toggle("Freno de Emergencia", value=False, disabled=not control_remoto)

st.sidebar.divider()
activar_camara = st.sidebar.toggle("Activar Camara YOLO", value=False)

# Aqui formateamos el mensaje de control remoto para el envio
estado = 1 if control_remoto else 0
freno = 1 if freno_remoto else 0
payload = f"{estado},{aceleracion},{freno}"

# Aqui transmitimos el mensaje via MQTT ante variaciones de estado
if "ultimo_payload" not in st.session_state:
    st.session_state.ultimo_payload = None

if payload != st.session_state.ultimo_payload:
    try:
        publish.single("tractor/control", payload, hostname="localhost")
        st.session_state.ultimo_payload = payload
    except Exception as e:
        st.sidebar.error(f"Error MQTT: {e}")

# Aqui estructuramos el contenido principal del panel
df = cargar_datos()
estado_seguridad = obtener_seguridad()

# Aqui mostramos las alertas visuales segun la presencia de obstaculos
if estado_seguridad == 1:
    st.markdown("<div style='background-color: #ff4b4b; padding: 10px; border-radius: 10px; text-align: center; margin-bottom: 20px;'><h2 style='color: white; margin: 0;'>OBSTACULO DETECTADO!</h2></div>", unsafe_allow_html=True)
else:
    st.markdown("<div style='background-color: #367C2B; padding: 10px; border-radius: 10px; text-align: center; margin-bottom: 20px;'><h2 style='color: white; margin: 0;'>CAMINO DESPEJADO</h2></div>", unsafe_allow_html=True)

if not df.empty:
    ultimo = df.iloc[-1]
    vel_motor = ultimo['Velocidad del motor']
    vel_vehiculo = ultimo['Velocidad del vehiculo']
    marcha_actual = int(ultimo['Marcha'])

    if activar_camara:
        col_marcha, col_cam = st.columns([1, 1])
        with col_marcha:
            st.markdown("<h4 style='text-align: center; color: gray; margin-bottom: 0px;'>Marcha</h4>", unsafe_allow_html=True)
            st.markdown(f"<h1 style='text-align: center; font-size: 8rem; color: #367C2B; margin-top: -30px; font-weight: bold;'>{marcha_actual}</h1>", unsafe_allow_html=True)
        with col_cam:
            st.markdown("<h4 style='text-align: center; color: gray; margin-bottom: 10px;'>Vision YOLO en Vivo</h4>", unsafe_allow_html=True)
            # Aqui renderizamos el visor de video en tiempo real alimentado por YOLO
            st.components.v1.iframe("http://192.168.137.160:5000/video_feed", width=500, height=380)
    else:
        # Renderizamos la marcha del vehiculo
        st.markdown("<h4 style='text-align: center; color: gray; margin-bottom: 0px;'>Marcha</h4>", unsafe_allow_html=True)
        st.markdown(f"<h1 style='text-align: center; font-size: 8rem; color: #367C2B; margin-top: -30px; font-weight: bold;'>{marcha_actual}</h1>", unsafe_allow_html=True)
    
    st.divider()
    
    # Aqui generamos los medidores circulares para RPM y velocidad
    col_taco1, col_taco2 = st.columns(2)
    
    with col_taco1:
        fig_rpm = crear_tacometro(vel_motor, "Tacometro Motor", "RPM", 5200, 4400, "#367C2B")
        st.plotly_chart(fig_rpm, use_container_width=True, key="rpm") 
        
    with col_taco2:
        fig_vel = crear_tacometro(vel_vehiculo, "Tacometro Vehiculo", "km/h", 120, 100, "#367C2B")
        st.plotly_chart(fig_vel, use_container_width=True, key="vel")
        
    st.divider()
    
    # Aqui generamos los graficos de linea para mostrar la evolucion temporal
    col_graf1, col_graf2 = st.columns(2)
    
    df_plot = df.reset_index(drop=True).reset_index()
    df_plot.rename(columns={'index': 'Muestra (historico)'}, inplace=True)

    with col_graf1:
        st.subheader("Tendencia del Motor")
        # Generamos el grafico de la evolucion de las revoluciones
        chart_rpm = alt.Chart(df_plot).mark_line(color="#FFDE00").encode(
            x=alt.X('Muestra (historico)', title='Punto de Muestreo (historico)'),
            y=alt.Y('Velocidad del motor', title='Revoluciones por Minuto (RPM)')
        ).properties(title="Evolucion de RPM")
        st.altair_chart(chart_rpm, use_container_width=True)
        
    with col_graf2:
        st.subheader("Tendencia del Vehiculo")
        # Generamos el grafico de la evolucion de la velocidad
        chart_vel = alt.Chart(df_plot).mark_line(color="#367C2B").encode(
            x=alt.X('Muestra (historico)', title='Punto de Muestreo (historico)'),
            y=alt.Y('Velocidad del vehiculo', title='Velocidad Lineal (km/h)')
        ).properties(title="Evolucion de Velocidad")
        st.altair_chart(chart_vel, use_container_width=True)
        
else:
    st.info("Esperando datos de la ESP32 en InfluxDB...")
    
# Aqui controlamos el tiempo de espera entre actualizaciones
time.sleep(0.5) 
try:
    st.rerun()
except AttributeError:
    st.experimental_rerun()
