from kubernetes import config, client

config.load_kube_config(config_file="./aws.yaml", context="bedrock")

v1 = client.CoreV1Api()

ret = v1.list_pod_for_all_namespaces(watch=False)

for i in ret.items:
    print(f"{i.status.pod_ip}, {i.metadata.namespace}, {i.metadata.name}")