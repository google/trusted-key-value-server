# Key/Value Server

[TOC]

## Disclaimer

This is not an officially supported Google product.

Teams from across Google, including Ads teams, are actively engaged in industry dialog about new technologies that can ensure a healthy ecosystem and preserve core business models. Online discussions (e.g. on GitHub) of technology proposals should not be interpreted as commitments about Google ads products.

## Overview

This is a simple key/value server, for use as part of the [FLEDGE
explainer](https://github.com/WICG/turtledove/blob/main/FLEDGE.md).  It is
designed to be able to act as one of the "trusted servers" that FLEDGE proposes
for buyers and sellers.

It was written for exploration purposes and so that we can run load tests using
it.

### Use

This is an in-memory hash map that's backed by a Google Cloud Spanner
database.  The data is loaded once on startup and refreshed every n minutes.

## Maintenance

This code is published so that it's possible for anyone to re-run the load tests
that we're doing.  This code will not be supported once the load testing is
complete.
