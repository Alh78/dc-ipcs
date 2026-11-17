# DC-IPCS

*A minimal IPC suntime scheduler that requires no internet connection.*


## To Get Started

DC-IPCS is a simple and minimal app responsible for frequently setting the suntime and sunrise schedules of Dahua IPCs, requiring no access to any external network.

You can either download the latest release or compile the binary file yourself. After these steps, in order to use this program effectively, it is suggested to follow the guide under the [How to Use](#How-to-Use) title.

### Downloading

When pre-compiled files are released, you'll be able to download it from [here](releases/).

---

### Compiling

To compile the binary, follow the steps below:

- **Cloning the repository**:

```bash
git clone --recurse-submodules https://github.com/Alh78/dc-ipcs.git
```

- **Compiling the binary**:

```bash
cmake -B build/
```

## How to Use

This app mainly works based on the values it reads from the config file ([`assets/config.json`](assets/config.json)). After you clone the repository, you can modify this config file according to your needs. Properties in the config file are listed and explained below:

### Attributes Defined in the Config File

- `ipc_addresses`: A list of IP addresses to be updated frequently. Even if there is only one IPC address, the list structure should *never* be **changed** to *any other type*, such as a string, as the app expects *only the list type* for this attribute.

- `credentials`: A dictionary which includes the required username and password to log in to the IPCs and send schedule requests. As mentioned above, the types of the values should **remain** *as they are* in the config file. It is **strongly recommended** to create *a new account* dedicated to this app on the relevant IPCs with the minimum required privileges, which is explained further in the related note.

- `latitude` & `longtitude`: Location coordinates where your IPCs are placed. You may also use *the approximate location* of your city, town, or street *(which is recommended for privacy)* if you do not wish to be precise.

- `elevation` *(optional)*: If you would like to improve the accuracy of the time calculation, you may add this variable with an integer value.

- `reattempt_delay`: The amount of time the app will wait if a previous request fails.

- `update_interval`: The time gap between each update of the schedule, which improves efficiency.

- `ls_data_file_path`: The path of the file where the app writes the timestamp of each update and reads the last time the schedule was updated.

- `cert_path` **(Still in development, doesn't work yet)**: If HTTPS is preferred in your setup, you can pass your certificate by writing the file path of the certificate file.

---


Just like they are placed *in the same folder as the binary file* during the *compilation* process, don't forget to **place** `config.json` *in the same directory as the binary file* if you downloaded it from the Releases page. It is also *important* to **run the app** *from its own directory*; otherwise, it'll **fail** to find *the config file* and abort.



*The steps* and *the necessary files* to add this app as a *`cron job`* in *Linux* and as a *`scheduled task`* in *Windows* will be added *later*, as the project is *still* under *development*.



**Note:** It is strongly advised to create a fresh account exclusively for this app with the fewest privileges possible. If you use your admin account or a different account with broad permissions, because the credentials are stored unencrypted on the disk, there will be a huge security risk depending on the system the app is running on, which may result in surveillance or inclusion in botnets. However, encryption for the credentials is in development, but this does not invalidate the danger of this situation.

## Contributing

You can read [this page](/CONTRIBUTING.md) if you consider to contribute to this project.

## License

This repository is under the [GPL-3.0 License.](/LICENSE.md)
