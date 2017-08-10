/*
 * Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
 * Weill Cornell Medical College
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
============================================================
Excitatory and Inhibitory Conductance-Based Synaptic Activity 
  driven by Poisson Shotnoise input
============================================================
 */

#include "exc-inh-shotnoise-input.h"
#include <iostream>
#include <main_window.h>

double ExcInhShotnoiseInput::RdmNumber() {
  double rdm_number;
  /* (nastyly generated) uniformily distributed random number */  
  rdm_number = (rand() % 1000000) * 1e-6 ; 
  return rdm_number;
}

void ExcInhShotnoiseInput::conductance_update(double* Ge, double* Gi) 
{
  *Ge *= exp(-period/steps/(1e-3*Te)) ; 
  *Gi *= exp(-period/steps/(1e-3*Ti)) ; 
  if (RdmNumber()<Ne*Fe*period/steps) { *Ge += Qe;} // kept in nS here
  if (RdmNumber()<Ni*Fi*period/steps) { *Gi += Qi;}
}

extern "C" Plugin::Object*
createRTXIPlugin(void)
{
  return new ExcInhShotnoiseInput();
}

static DefaultGUIModel::variable_t vars[] =
  {
    { "Vm", "Membrane potential (V)", DefaultGUIModel::INPUT, },
    { "Isyn (A)", "Output current (A)", DefaultGUIModel::OUTPUT, },
    { "Qe (nS)", "Exc. Synaptic Weight (nS)",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Qi (nS)", "Inh. Synaptic Weight (nS)",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Te (ms)", "Exc. Synaptic Time constant (ms)",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Ti (ms)", "Inh. Synaptic Time constant (ms)",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Ee (mV)", "Exc. Reversal Potential (mV)",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Ei (mV)", "Inh. Reversal Potential (mV)",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Ne", "Exc. Syn. Number",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Ni", "Inh. Syn. Number",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Fe (Hz)", "Exc. Freq. of Poisson Process",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Fi (Hz)", "Inh. Freq. of Poisson Process",
      DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Rate (Hz)", "Rate of integration (Hz)", DefaultGUIModel::PARAMETER
      | DefaultGUIModel::UINTEGER, },
    { "Time (s)", "Time (s)", DefaultGUIModel::STATE, },
  };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);


ExcInhShotnoiseInput::ExcInhShotnoiseInput(void)
  : DefaultGUIModel("Exc-Inh Shotnoise-Input", ::vars, ::num_vars)
{
  initParameters();
  setWhatsThis("<p><b>ExcInhShotnoiseInput:</b><br>Excitatory and Inhibitory Conductance-Based Synaptic Activity driven by Poisson Shotnoise input</p>");
  DefaultGUIModel::createGUI(vars,
                             num_vars); // this is required to create the GUI
  update(INIT); // this is optional, you may place initialization code directly
                // into the constructor
  refresh();    // this is required to update the GUI with parameter and state
                // values
  QTimer::singleShot(0, this, SLOT(resizeMe()));
}

ExcInhShotnoiseInput::~ExcInhShotnoiseInput(void)
{
}

void
ExcInhShotnoiseInput::execute(void)
{
  systime = count * period; // time in seconds
  Vm = input(0); // voltage in V
  for (int i = 0; i < steps; ++i)
    {
      conductance_update(&Ge, &Gi) ;
    }
  /* === HERE WE GET BACK TO SI UNITS === */
  output(0) = (Ge*1e-9)*(Ee*1e-3-Vm)+(Gi*1e-9)*(Ei*1e-3-Vm);
  count++;
}

void
ExcInhShotnoiseInput::initParameters(void)
{
  Ge = 0.0; Gi = 0.0; // nS
  Qe = 2.0; Qi = 10.0; // nS
  Te = 5.0; Ti = 5.0; // ms
  Ee = 0.0; Ei = -80.0; // mV
  Ne = 2000; Ni = 500;
  Fe = 4.0; Fi = 7.0; // Hz
  Vm = -70.0 ;
  systime = 0.0;
  count = 0;
  rate = 20000.0; // 20kHz per default here
  period = RT::System::getInstance()->getPeriod() * 1e-9; // s
  steps = static_cast<int> (ceil(period * rate));  
}

void
ExcInhShotnoiseInput::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      setParameter("Qe (nS)", QString::number(Qe)); 
      setParameter("Qi (nS)", QString::number(Qi));
      setParameter("Ee (mV)", QString::number(Ee));
      setParameter("Ei (mV)", QString::number(Ei));
      setParameter("Te (ms)", QString::number(Te)); 
      setParameter("Ti (ms)", QString::number(Ti));
      setParameter("Ne", QString::number(Ne));
      setParameter("Ni", QString::number(Ni));
      setParameter("Fe (Hz)", QString::number(Fe));
      setParameter("Fi (Hz)", QString::number(Fi));
      setParameter("Rate (Hz)", QString::number(rate)); 
      setState("Time (s)", systime);
      period = RT::System::getInstance()->getPeriod() * 1e-9; // s
      steps = static_cast<int> (ceil(period * rate));
      break;

    case MODIFY:
      Qe = getParameter("Qe (nS)").toDouble();
      Qi = getParameter("Qi (nS)").toDouble();
      Ee = getParameter("Ee (mV)").toDouble();
      Ei = getParameter("Ei (mV)").toDouble();
      Te = getParameter("Te (ms)").toDouble(); 
      Ti = getParameter("Ti (ms)").toDouble(); 
      Ne = getParameter("Ne").toInt(); 
      Ni = getParameter("Ni").toInt(); 
      Fe = getParameter("Fe (Hz)").toDouble(); 
      Fi = getParameter("Fi (Hz)").toDouble(); 
      rate = getParameter("Rate (Hz)").toDouble();
      steps = static_cast<int> (ceil(period * rate));
      Ge = 0; Gi = 0; // nS
      break;

    case UNPAUSE:
      break;

    case PAUSE:
      break;

    case PERIOD:
      period = RT::System::getInstance()->getPeriod() * 1e-9; // s
      steps = static_cast<int> (ceil(period * rate));
      break;

    default:
      break;
  }
}

