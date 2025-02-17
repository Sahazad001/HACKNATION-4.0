# -*- coding: utf-8 -*-
"""Falcon_ML.ipynb

Automatically generated by Colab.

Original file is located at
    https://colab.research.google.com/drive/1KCuRnW7kZ09FR0rWwhDP8RkMilK8jwP0
"""


import pandas as pd

# Load the dataset
file_path = "weather_data.csv"
df = pd.read_csv(file_path)

# Display basic information and first few rows
df.info(), df.head()

# Convert 'Date' column to datetime format
df['Formatted Date'] = pd.to_datetime(df['Formatted Date'])

# Check for missing values
missing_values = df.isnull().sum()

# Display summary
df.describe(), missing_values

# Feature Engineering: Create Lag Features for Past 3 Days
for lag in range(1, 4):
    df[f'Temp_Lag_{lag}'] = df['Temperature (C)'].shift(lag)
    df[f'Humidity_Lag_{lag}'] = df['Humidity'].shift(lag)
    df[f'Wind_Lag_{lag}'] = df['Wind Speed (km/h)'].shift(lag)
    df[f'Pressure_Lag_{lag}'] = df['Pressure (millibars)'].shift(lag)

# Moving Averages (Trend Features)
df['Temp_MA_3'] = df['Temperature (C)'].rolling(window=3).mean()
df['Humidity_MA_3'] = df['Humidity'].rolling(window=3).mean()
df['Wind_MA_3'] = df['Wind Speed (km/h)'].rolling(window=3).mean()
df['Pressure_MA_3'] = df['Pressure (millibars)'].rolling(window=3).mean()

# Convert 'Formatted Date' to datetime and set UTC
df['Formatted Date'] = pd.to_datetime(df['Formatted Date'], utc=True)

# Extract date features
df['Year'] = df['Formatted Date'].dt.year
df['Month'] = df['Formatted Date'].dt.month
df['Day'] = df['Formatted Date'].dt.day


# Drop initial rows with NaN due to shifting
df.dropna(inplace=True)

!pip install category_encoders

import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from xgboost import XGBRegressor
from sklearn.metrics import mean_absolute_error
from category_encoders import TargetEncoder # Import TargetEncoder




# Define target variable (predict next day's temperature)
target = 'Temperature (C)'

# Select features, excluding the target and date columns
features = [col for col in df.columns if col not in ['Formatted Date', target]]

# Handle categorical features using Target Encoding
categorical_features = ['Summary', 'Precip Type', 'Daily Summary'] # Identify categorical columns
encoder = TargetEncoder(cols=categorical_features) # Create a TargetEncoder object
df[categorical_features] = encoder.fit_transform(df[categorical_features], df[target]) # Encode categorical features

# Handle missing values by filling them with the column median
for col in df.select_dtypes(include=np.number):
    df[col] = df[col].fillna(df[col].median())

# Split data into train and test sets (80% training, 20% testing) without shuffling
X_train, X_test, y_train, y_test = train_test_split(df[features], df[target], test_size=0.2, random_state=42, shuffle=False)

# Train the XGBoost Regressor
model = XGBRegressor(n_estimators=500, learning_rate=0.05, max_depth=6, random_state=42)
model.fit(X_train, y_train)

# Predict and evaluate
y_pred = model.predict(X_test)
mae = mean_absolute_error(y_test, y_pred)
print(f"Mean Absolute Error: {mae}")

# Define farming advice function
def farming_advice(temp, humidity, wind, pressure):
    advice = []

    # Irrigation Management
    if temp > 30 and pressure > 1010:
        advice.append("Consider irrigating the crops due to high temperature and stable pressure conditions.")

    # Fertilizer Application
    if pressure < 1000:
        advice.append("Avoid fertilizer application due to potential unstable weather conditions.")
    else:
        advice.append("Fertilizer application is safe.")

    # Disease Prevention
    if humidity > 80:
        advice.append("High humidity detected! Monitor for fungal infections and consider fungicide application.")

    # Harvesting Decision
    if wind > 15:
        advice.append("Avoid harvesting due to strong winds.")
    else:
        advice.append("Weather is suitable for harvesting.")

    return advice

# Get latest test data row for predictions
latest_test_features = X_test.iloc[-1]
latest_pred_temp = y_pred[-1]

# Extract relevant values for farming advice
humidity = latest_test_features.get('Humidity', 50)
wind = latest_test_features.get('Wind Speed (km/h)', 5)
pressure = latest_test_features.get('Pressure (millibars)', 1010)

# Generate and print farming advice
advice = farming_advice(latest_pred_temp, humidity, wind, pressure)
print("\nFarming Advice:")
for a in advice:
    print("-", a)

