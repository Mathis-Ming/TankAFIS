import asyncio
from bleak import BleakClient

# Remplacez ceci par l'adresse MAC de votre périphérique BLE
ADDRESS = "70:3E:97:E3:65:1B"
# Remplacez ceci par l'UUID de la caractéristique Notify
NOTIFY_CHAR_UUID = "0000FFE2-0000-1000-8000-00805F9B34FB"

def notification_handler(sender, data):
    print("Données reçues: ", data.decode('utf-8'))

async def main(address, char_uuid):
    client = BleakClient(address)
    try:
        await client.connect()
        print("Connecté.")
        await client.start_notify(char_uuid, notification_handler)
        print("Notification activée. En attente de données...")
        await asyncio.sleep(30)  # Modifier si nécessaire pour attendre plus ou moins longtemps
        await client.stop_notify(char_uuid)
        print("Notification désactivée.")
        await client.disconnect()
        print("Déconnecté.")
    except Exception as e:
        print(e)

if __name__=="__main__":
    asyncio.run(main(ADDRESS, NOTIFY_CHAR_UUID))