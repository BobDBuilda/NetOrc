const { faker } = require('@faker-js/faker');

/**
 * Mimics SNMP Telemetry data structure.
 * SNMP data is usually hierarchical (OIDs) and time-stamped.
 */
function generateSNMPMetric() {
    const devices = ['core-switch-01', 'edge-router-04', 'distribution-layer-02'];
    const interfaces = ['GigabitEthernet0/1', 'TenGigabitEthernet1/1', 'eth0'];
    const oids = [
        { name: 'ifInOctets', oid: '1.3.6.1.2.1.2.2.1.10' },
        { name: 'ifOutOctets', oid: '1.3.6.1.2.1.2.2.1.16' },
        { name: 'ifInErrors', oid: '1.3.6.1.2.1.2.2.1.14' },
        { name: 'sysUpTime', oid: '1.3.6.1.2.1.1.3.0' }
    ];

    const selectedOid = faker.helpers.arrayElement(oids);

    return {
        timestamp: new Date().toISOString(),
        device_id: faker.helpers.arrayElement(devices),
        ip_address: faker.internet.ip(),
        metric_name: selectedOid.name,
        oid: selectedOid.oid,
        interface: faker.helpers.arrayElement(interfaces),
        value: faker.number.int({ min: 100, max: 1000000 }),
        unit: 'counts/octets',
        status: faker.helpers.arrayElement(['up', 'up', 'up', 'down', 'congested']) // Weight 'up'
    };
}

// Generate a batch of 10 metrics
const telemetryBatch = Array.from({ length: 10 }, generateSNMPMetric);

console.log(JSON.stringify(telemetryBatch, null, 2));

/**
 * Regarding your question: "Do SNMP data get stored like this?"
 * 
 * Yes, but with one major difference:
 * In high-performance systems, we use "Normalization":
 * 1. A 'Devices' table stores the IP/Name.
 * 2. An 'OIDs' table stores the long string (1.3.6.1...) mapped to a name.
 * 3. The 'Telemetry' table only stores IDs (integers) and the value.
 * 
 * This keeps the database small while handling millions of rows per hour.
 */
