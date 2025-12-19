import sys
import json
import pickle
import numpy as np
import os
import warnings

warnings.filterwarnings("ignore")

from dotenv import load_dotenv

load_dotenv()

def load_model():
    with open(os.getenv("MODEL_PATH"), 'rb') as f:
        model = pickle.load(f)

    with open(os.getenv("SCALER_PATH"), 'rb') as f:
        scaler = pickle.load(f)

    with open(os.getenv("METADATA_PATH"), 'r') as f:
        metadata = json.load(f)

    return model, scaler, metadata

def predict(features, model, scaler, uses_scaling):
    features_array = np.array(features).reshape(1, -1)

    if uses_scaling:
        features_scaled = scaler.transform(features_array)
        prediction = model.predict(features_scaled)[0]
        probability = model.predict_proba(features_scaled)[0]
    else:
        prediction = model.predict(features_array)[0]
        probability = model.predict_proba(features_array)[0]

    return int(prediction), float(probability[1])

def main():
    try:
        model, scaler, metadata = load_model()

        for line in sys.stdin:
            line = line.strip()
            if not line:
                continue

            if line == "EXIT":
                break

            if line == "INFO":
                response = {
                    'status': 'success',
                    'data': {
                        'model_name': metadata['best_model'],
                        'features': metadata['features'],
                        'num_features': metadata['num_features'],
                        'metrics': metadata['metrics']
                    }
                }
                print(json.dumps(response), flush=True)
                continue

            try:
                data = json.loads(line)
                features = data['features']

                prediction, probability = predict(
                    features,
                    model,
                    scaler,
                    metadata['uses_scaling']
                )

                response = {
                    'status': 'success',
                    'prediction': prediction,
                    'probability': probability
                }
                print(json.dumps(response), flush=True)

            except Exception as e:
                response = {
                    'status': 'error',
                    'message': str(e)
                }
                print(json.dumps(response), flush=True)

    except Exception as e:
        response = {
            'status': 'error',
            'message': f'initialization error: {str(e)}'
        }
        print(json.dumps(response), flush=True)
        sys.exit(1)

if __name__ == '__main__':
    main()