import React, { Component } from 'react';
import { Redirect, Switch, RouteComponentProps } from 'react-router-dom'

import { Tabs, Tab } from '@material-ui/core';

import { AuthenticatedContextProps, withAuthenticatedContext, AuthenticatedRoute } from '../authentication';
import { MenuAppBar } from '../components';
import MQTTStatusController from './MQTTStatusController';
import MQTTSettingsController from './MQTTSettingsController';

type AccessPointProps = AuthenticatedContextProps & RouteComponentProps;

class AccessPoint extends Component<AccessPointProps> {

  handleTabChange = (event: React.ChangeEvent<{}>, path: string) => {
    this.props.history.push(path);
  };

  render() {
    const { authenticatedContext } = this.props;
    return (
      <MenuAppBar sectionTitle="MQTT">
        <Tabs value={this.props.match.url} onChange={this.handleTabChange} variant="fullWidth">
          <Tab value="/mqtt/status" label="MQTT Status" />
          <Tab value="/mqtt/settings" label="MQTT Settings" disabled={!authenticatedContext.me.admin} />
        </Tabs>
        <Switch>
          <AuthenticatedRoute exact path="/mqtt/status" component={MQTTStatusController} />
          <AuthenticatedRoute exact path="/mqtt/settings" component={MQTTSettingsController} />
          <Redirect to="/mqtt/status" />
        </Switch>
      </MenuAppBar>
    )
  }
}

export default withAuthenticatedContext(AccessPoint);
