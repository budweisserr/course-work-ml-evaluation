import pandas as pd
import numpy as np
import os
from dotenv import load_dotenv
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, GradientBoostingClassifier
from sklearn.svm import SVC
from sklearn.neighbors import KNeighborsClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score, roc_auc_score, confusion_matrix, classification_report
import matplotlib.pyplot as plt
import seaborn as sns
import pickle

load_dotenv()

df = pd.read_csv(os.getenv("DATASET_PATH"))

print(df.head())
print(df.info())
print(df.describe())
print(df.isnull().sum())

plt.figure(figsize=(12, 8))
df.hist(bins=20, figsize=(15, 10))
plt.tight_layout()
plt.savefig('png/distributions.png')
plt.close()

plt.figure(figsize=(12, 10))
sns.heatmap(df.corr(), annot=True, cmap='coolwarm', fmt='.2f')
plt.title('correlation matrix')
plt.tight_layout()
plt.savefig('png/correlation.png')
plt.close()

target_counts = df['target'].value_counts()
plt.figure(figsize=(8, 6))
plt.bar(target_counts.index, target_counts.values)
plt.xlabel('target')
plt.ylabel('count')
plt.title('target distribution')
plt.savefig('png/target_distribution.png')
plt.close()

X = df.drop('target', axis=1)
y = df['target']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42, stratify=y)

scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

models = {
    'logistic_regression': LogisticRegression(max_iter=1000, random_state=42),
    'decision_tree': DecisionTreeClassifier(random_state=42),
    'random_forest': RandomForestClassifier(n_estimators=100, random_state=42),
    'gradient_boosting': GradientBoostingClassifier(n_estimators=100, random_state=42),
    'svm': SVC(probability=True, random_state=42),
    'knn': KNeighborsClassifier(),
    'naive_bayes': GaussianNB()
}

results = {}

for name, model in models.items():
    if name in ['svm', 'knn', 'logistic_regression']:
        model.fit(X_train_scaled, y_train)
        y_pred = model.predict(X_test_scaled)
        y_pred_proba = model.predict_proba(X_test_scaled)[:, 1]
    else:
        model.fit(X_train, y_train)
        y_pred = model.predict(X_test)
        y_pred_proba = model.predict_proba(X_test)[:, 1]

    accuracy = accuracy_score(y_test, y_pred)
    precision = precision_score(y_test, y_pred)
    recall = recall_score(y_test, y_pred)
    f1 = f1_score(y_test, y_pred)
    roc_auc = roc_auc_score(y_test, y_pred_proba)
    cm = confusion_matrix(y_test, y_pred)

    results[name] = {
        'model': model,
        'accuracy': accuracy,
        'precision': precision,
        'recall': recall,
        'f1': f1,
        'roc_auc': roc_auc,
        'confusion_matrix': cm,
        'predictions': y_pred
    }

    print(f'\n{name}:')
    print(f'accuracy: {accuracy:.4f}')
    print(f'precision: {precision:.4f}')
    print(f'recall: {recall:.4f}')
    print(f'f1 score: {f1:.4f}')
    print(f'roc auc: {roc_auc:.4f}')
    print(f'confusion matrix:\n{cm}')

    plt.figure(figsize=(6, 5))
    sns.heatmap(cm, annot=True, fmt='d', cmap='Blues')
    plt.title(f'{name} confusion matrix')
    plt.ylabel('actual')
    plt.xlabel('predicted')
    plt.tight_layout()
    plt.savefig(f'png/{name}_confusion_matrix.png')
    plt.close()

results_df = pd.DataFrame({
    'model': list(results.keys()),
    'accuracy': [results[k]['accuracy'] for k in results.keys()],
    'precision': [results[k]['precision'] for k in results.keys()],
    'recall': [results[k]['recall'] for k in results.keys()],
    'f1': [results[k]['f1'] for k in results.keys()],
    'roc_auc': [results[k]['roc_auc'] for k in results.keys()]
})

results_df = results_df.sort_values('recall', ascending=False)
print('\ncomparison of all models:')
print(results_df.to_string(index=False))

fig, axes = plt.subplots(2, 3, figsize=(15, 10))
metrics = ['accuracy', 'precision', 'recall', 'f1', 'roc_auc']
for idx, metric in enumerate(metrics):
    ax = axes[idx // 3, idx % 3]
    sorted_data = results_df.sort_values(metric, ascending=False)
    ax.barh(sorted_data['model'], sorted_data[metric])
    ax.set_xlabel(metric)
    ax.set_xlim(0, 1)
    for i, v in enumerate(sorted_data[metric]):
        ax.text(v + 0.01, i, f'{v:.3f}', va='center')
axes[1, 2].axis('off')
plt.tight_layout()
plt.savefig('png/metrics_comparison.png')
plt.close()

best_model_name = results_df.iloc[0]['model']
best_model = results[best_model_name]['model']

print(f'\nbest model selected: {best_model_name}')
print(f'best recall score: {results[best_model_name]["recall"]:.4f}')

with open('best_model.pkl', 'wb') as f:
    pickle.dump(best_model, f)

with open('scaler.pkl', 'wb') as f:
    pickle.dump(scaler, f)

with open('model_info.txt', 'w') as f:
    f.write(f'best model: {best_model_name}\n')
    f.write(f'accuracy: {results[best_model_name]["accuracy"]:.4f}\n')
    f.write(f'precision: {results[best_model_name]["precision"]:.4f}\n')
    f.write(f'recall: {results[best_model_name]["recall"]:.4f}\n')
    f.write(f'f1 score: {results[best_model_name]["f1"]:.4f}\n')
    f.write(f'roc auc: {results[best_model_name]["roc_auc"]:.4f}\n')
    f.write(f'uses scaling: {best_model_name in ["svm", "knn", "logistic_regression"]}\n')

print('\nmodel saved as best_model.pkl')
print('scaler saved as scaler.pkl')
print('model info saved as model_info.txt')